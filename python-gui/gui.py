
from flask import Flask, jsonify
from flask_cors import CORS
import socket
import threading
import time
import re
import tkinter as tk
from tkinter import ttk
from ttkthemes import ThemedTk

app = Flask(__name__)

# 启用 CORS，允许所有来源访问
CORS(app, resources={r"/*": {"origins": "*"}})

# 全局变量存储温度和湿度数据
temperature = 0.0
humidity = 0.0
connection_status = '未连接'
last_data = '无数据'

# 添加线程锁以确保线程安全
lock = threading.Lock()

# 服务器信息
server_host = '118.178.143.157'
server_port = 8002

# 创建TCP套接字
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(5)

def connect_to_server():
    """连接到服务器"""
    global connection_status
    try:
        sock.connect((server_host, server_port))
        with lock:
            connection_status = '已连接'
        print(f"成功连接到服务器 {server_host}:{server_port}")
        return True
    except socket.error as e:
        with lock:
            connection_status = f'连接失败: {str(e)}'
        print(f"连接失败: {str(e)}")
        return False
    except Exception as e:
        with lock:
            connection_status = f'错误: {str(e)}'
        print(f"连接错误: {str(e)}")
        return False

def receive_temperature():
    """接收温度和湿度数据"""
    global temperature, humidity, connection_status, last_data

    # 尝试连接服务器
    if not connect_to_server():
        print("初始连接失败，退出接收线程")
        return

    while True:
        try:
            data = sock.recv(1024).decode('utf-8')
            if not data:
                with lock:
                    connection_status = '服务器断开'
                print("服务器断开连接，尝试重新连接...")
                if not connect_to_server():
                    time.sleep(5)
                continue

            with lock:
                last_data = data  # 保存最后收到的数据
            print(f"收到数据: {repr(data)}")  # 打印原始数据用于调试

            # 使用正则表达式提取温度值
            match_temp = re.search(r'Tem:\s*([\d.]+)C', data)
            if match_temp:
                try:
                    with lock:
                        temperature = float(match_temp.group(1))  # 更新全局温度变量
                    print(f"更新温度: {temperature}°C")
                except Exception as e:
                    print(f"温度解析错误: {str(e)}")

            # 使用正则表达式提取湿度值
            match_hum = re.search(r'Hum:\s*([\d.]+)C', data)
            if match_hum:
                try:
                    with lock:
                        humidity = float(match_hum.group(1))  # 更新全局湿度变量
                    print(f"更新湿度: {humidity}%")
                except Exception as e:
                    print(f"湿度解析错误: {str(e)}")

        except socket.timeout:
            continue
        except socket.error as e:
            print(f"套接字错误: {str(e)}")
            with lock:
                connection_status = f'套接字错误: {str(e)}'
            if not connect_to_server():
                time.sleep(5)
        except Exception as e:
            print(f"接收错误: {str(e)}")
            with lock:
                connection_status = f'错误: {str(e)}'
            time.sleep(1)

@app.route('/get_temperature')
def get_temperature():
    """获取最新温度和湿度数据"""
    with lock:
        print(f"返回温度: {temperature}°C, 湿度: {humidity}%")  # 调试输出
        return jsonify({
            'temperature': temperature,
            'humidity': humidity,
            'connection': connection_status,
            'last_data': last_data
        })

def create_gui():
    """创建并运行 tkinter GUI"""
    # 使用 ThemedTk 应用现代主题
    root = ThemedTk(theme="arc")  # 使用 'arc' 主题，简洁现代
    root.title("温度与湿度监控")
    root.geometry("450x300")
    root.configure(bg="#f5f6f5")  # 设置背景色，浅灰色更现代

    # 创建主框架
    main_frame = ttk.Frame(root, padding=20)
    main_frame.pack(fill="both", expand=True)

    # 创建标题标签
    title_label = ttk.Label(
        main_frame,
        text="温度与湿度监控",
        font=("Helvetica", 18, "bold"),
        anchor="center"
    )
    title_label.pack(pady=(0, 20))

    # 创建数据展示框架
    data_frame = ttk.Frame(main_frame)
    data_frame.pack(fill="x", padx=10)

    # 温度显示
    temp_frame = ttk.Frame(data_frame)
    temp_frame.pack(fill="x", pady=5)
    ttk.Label(
        temp_frame,
        text="温度",
        font=("Helvetica", 12),
        width=10
    ).pack(side="left")
    temperature_label = ttk.Label(
        temp_frame,
        text="0.0°C",
        font=("Helvetica", 12, "bold"),
        foreground="#2c3e50"
    )
    temperature_label.pack(side="left", padx=10)

    # 湿度显示
    hum_frame = ttk.Frame(data_frame)
    hum_frame.pack(fill="x", pady=5)
    ttk.Label(
        hum_frame,
        text="湿度",
        font=("Helvetica", 12),
        width=10
    ).pack(side="left")
    humidity_label = ttk.Label(
        hum_frame,
        text="0.0%",
        font=("Helvetica", 12, "bold"),
        foreground="#2c3e50"
    )
    humidity_label.pack(side="left", padx=10)

    # 连接状态显示
    conn_frame = ttk.Frame(data_frame)
    conn_frame.pack(fill="x", pady=5)
    ttk.Label(
        conn_frame,
        text="连接状态",
        font=("Helvetica", 12),
        width=10
    ).pack(side="left")
    connection_label = ttk.Label(
        conn_frame,
        text="未连接",
        font=("Helvetica", 12, "bold"),
        foreground="#e74c3c"  # 红色表示未连接
    )
    connection_label.pack(side="left", padx=10)

    # 最后数据
    data_frame_last = ttk.Frame(data_frame)
    data_frame_last.pack(fill="x", pady=5)
    ttk.Label(
        data_frame_last,
        text="最后数据",
        font=("Helvetica", 12),
        width=10
    ).pack(side="left")
    last_data_label = ttk.Label(
        data_frame_last,
        text="无数据",
        font=("Helvetica", 12, "bold"),
        foreground="#2c3e50"
    )
    last_data_label.pack(side="left", padx=10)

    def update_gui():
        """更新 GUI 显示"""
        with lock:
            temperature_label.config(text=f"{temperature}°C")
            humidity_label.config(text=f"{humidity}%")
            connection_label.config(
                text=connection_status,
                foreground="#27ae60" if connection_status == "已连接" else "#e74c3c"
            )
            last_data_label.config(text=last_data)
        root.after(1000, update_gui)  # 每秒更新一次

    # 启动 GUI 更新
    update_gui()

    # 运行 GUI 主循环
    root.mainloop()

if __name__ == '__main__':
    # 启动数据接收线程
    recv_thread = threading.Thread(target=receive_temperature, daemon=True)
    recv_thread.start()

    # 启动 Flask 线程
    flask_thread = threading.Thread(target=lambda: app.run(host='0.0.0.0', port=5000, debug=True, use_reloader=False), daemon=True)
    flask_thread.start()

    # 启动 GUI（主线程）
    create_gui()

