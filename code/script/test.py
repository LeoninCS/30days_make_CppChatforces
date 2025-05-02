import socket
import json

# 构造注册请求
request = {
    "msgid": 2,
    "id": 3,
    "password": "123456"
}

# 将 JSON 转为字符串（加 \n 更容易触发对方解析）
request_str = json.dumps(request) + "\n"

# 建立 TCP 连接
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(("127.0.0.1", 8888))  # 服务器地址和端口

# 发送请求
client.sendall(request_str.encode())

# 接收回应（最多 1024 字节）
response = client.recv(1024)
print("服务器回应:", response.decode())

client.close()
