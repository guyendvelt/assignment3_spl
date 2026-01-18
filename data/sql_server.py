#!/usr/bin/env python3
"""
Basic Python Server for STOMP Assignment – Stage 3.3

IMPORTANT:
DO NOT CHANGE the server name or the basic protocol.
Students should EXTEND this server by implementing
the methods below.
"""

import socket
import sys
import threading
import sqlite3
import socket
import os


SERVER_NAME = "STOMP_PYTHON_SQL_SERVER"  # DO NOT CHANGE!
DB_FILE = "stomp_server.db"              # DO NOT CHANGE!


def recv_null_terminated(sock: socket.socket) -> str:
    data = b""
    while True:
        chunk = sock.recv(1024)
        if not chunk:
            return ""
        data += chunk
        if b"\0" in data:
            msg, _ = data.split(b"\0", 1)
            return msg.decode("utf-8", errors="replace")


def print_server_report():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    
    print("\n--- SERVER REPORT ---")
    
    # 1. Registered Users
    print("1. Registered Users:")
    cursor.execute("SELECT username FROM Users")
    for row in cursor.fetchall():
        print(f"   - {row[0]}")
        
    # 2. Login History
    print("\n2. Login History:")
    cursor.execute("SELECT username, login_time, logout_time FROM Logins")
    for row in cursor.fetchall():
        logout = row[2] if row[2] else "Active"
        print(f"   - User: {row[0]}, Login: {row[1]}, Logout: {logout}")

    # 3. Uploaded Files
    print("\n3. Uploaded Files:")
    cursor.execute("SELECT username, filename FROM Files")
    for row in cursor.fetchall():
        print(f"   - User: {row[0]}, File: {row[1]}")
        
    print("---------------------\n")
    conn.close()


def init_database():
    print(f'{SERVER_NAME} Initializing database...')
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    #Create Users Table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS Users (
            username TEXT PRIMARY KEY,
            password TEXT NOT NULL
        )
        """)
        #Create Logins Table 
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS Logins (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            login_time TEXT NOT NULL,
            logout_time TEXT,
            FOREIGN KEY(username) REFERENCES Users(username)
        )
    """)
    #Create Files Tables
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS Files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            filename TEXT NOT NULL,
            upload_time TEXT,
            FOREIGN KEY(username) REFERENCES Users(username)
        )
    """)
    conn.commit()
    conn.close()
    print("Database initialized successfuly")



def execute_sql_command(sql_command: str) -> str:
    try:
        with sqlite3.connect(DB_FILE) as conn:
            cursor = conn.cursor();
            cursor.execute(sql_command)
            conn.commit()
        return "done"
    except sqlite3.Error as e:
        return f'SQL ERROR: {e}' 

        


def execute_sql_query(sql_query: str) -> str:
   try:
        with sqlite3.connect(DB_FILE) as conn:
            cursor = conn.cursor()
            cursor.execute(sql_query)
            rows = cursor.fetchall()
            if not rows:
                return ""
            response_lines = []
            for row in rows:
                response_lines.append(" ".join(str(item) for item in row))
            return "|".join(response_lines)
   except sqlite3.Error as e:
    return f'SQL ERROR: {e}'



def handle_client(client_socket: socket.socket, addr):
    print(f"[{SERVER_NAME}] Client connected from {addr}")

    try:
        while True:
            message = recv_null_terminated(client_socket)
            if message == "":
                break

            print(f"[{SERVER_NAME}] Received:")
            print(message)
            response  = ""
            clean_msg = message.strip().upper()
            if clean_msg == "REPORT":
                print_server_report()
                response = "Report printed on server console"
            elif clean_msg.startswith("SELECT"):
                response = execute_sql_query(message)
            else:
                response = execute_sql_command(message)
            client_socket.sendall(response.encode('utf-8') +  b"\0")
    except Exception as e:
        print(f"[{SERVER_NAME}] Error handling client {addr}: {e}")
    finally:
        try:
            client_socket.close()
        except Exception:
            pass
        print(f"[{SERVER_NAME}] Client {addr} disconnected")

def start_server(host="127.0.0.1", port=7778):
    init_database()
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server_socket.bind((host, port))
        server_socket.listen(5)
        print(f"[{SERVER_NAME}] Server started on {host}:{port}")
        print(f"[{SERVER_NAME}] Waiting for connections...")

        while True:
            client_socket, addr = server_socket.accept()
            t = threading.Thread(
                target=handle_client,
                args=(client_socket, addr),
                daemon=True
            )
            t.start()

    except KeyboardInterrupt:
        print(f"\n[{SERVER_NAME}] Shutting down server...")
    finally:
        try:
            server_socket.close()
        except Exception:
            pass


if __name__ == "__main__":
    port = 7778
    if len(sys.argv) > 1:
        raw_port = sys.argv[1].strip()
        try:
            port = int(raw_port)
        except ValueError:
            print(f"Invalid port '{raw_port}', falling back to default {port}")

    start_server(port=port)
