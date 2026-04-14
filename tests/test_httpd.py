import subprocess as sp
import socket
import requests
import time
import os

HOST = "127.0.0.1"
PORT = "8080"
EXECUTABLE = "./httpd"
ROOT_DIR = "./tests/test_root"

def spawn_httpd():
    os.makedirs(ROOT_DIR, exist_ok=True)
    proc = sp.Popen(
        [EXECUTABLE, "--pid_file", "/tmp/httpd_test.pid", "--ip", HOST,
         "--port", PORT, "--root_dir", ROOT_DIR,
         "--server_name", "test"],
        stdout=sp.PIPE,
        stderr=sp.PIPE
    )
    time.sleep(1)
    return proc

def kill_httpd(proc):
    proc.kill()
    proc.wait()

def test_200():
    os.makedirs(ROOT_DIR, exist_ok=True)
    with open(f"{ROOT_DIR}/index.html", "w") as f:
        f.write("<h1>Test Page</h1>")

    proc = spawn_httpd()
    try:
        req = requests.get(f"http://{HOST}:{PORT}/index.html")
        assert req.status_code == 200
        assert req.text == "<h1>Test Page</h1>"
    finally:
        kill_httpd(proc)

def test_404():
    proc = spawn_httpd()
    try:
        req = requests.get(f"http://{HOST}:{PORT}/doesntexist.html")
        assert req.status_code == 404
    finally:
        kill_httpd(proc)

def test_405():
    proc = spawn_httpd()
    try:
        req = requests.post(f"http://{HOST}:{PORT}/")
        assert req.status_code == 405
    finally:
        kill_httpd(proc)

def test_505():
    proc = spawn_httpd()
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, int(PORT)))

        request = f"GET / HTTP/1.0\r\nHost: {HOST}\r\n\r\n"
        sock.sendall(request.encode())

        resp = sock.recv(1024).decode()
        sock.close()

        assert "505" in resp.split('\r\n')[0]
    finally:
        kill_httpd(proc)

def test_400():
    proc = spawn_httpd()
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, int(PORT)))

        request = f"GET / HTTP/1.1\r\n\r\n"
        sock.sendall(request.encode())

        resp = sock.recv(1024).decode()
        sock.close()

        assert "400" in resp.split('\r\n')[0]
    finally:
        kill_httpd(proc)

def test_403():
    noperm = f"{ROOT_DIR}/noperm.html"
    with open(noperm, "w") as f:
        f.write("test")
    os.chmod(noperm, 0o000)

    proc = spawn_httpd()
    try:
        req = requests.get(f"http://{HOST}:{PORT}/noperm.html")
        assert req.status_code == 403
    finally:
        kill_httpd(proc)
        os.remove(noperm)

def test_head():
    with open(f"{ROOT_DIR}/test.html", "w") as f:
        f.write("test")

    proc = spawn_httpd()
    try:
        req = requests.head(f"http://{HOST}:{PORT}/test.html")

        assert req.status_code == 200
        assert "Date" in req.headers
        assert "Content-Length" in req.headers
        assert req.headers["Connection"] == "close"
        assert req.headers["Content-Length"] == "4"
        assert req.text == ""
    finally:
        kill_httpd(proc)

def test_default_file():
    with open(f"{ROOT_DIR}/index.html", "w") as f:
        f.write("default page")

    proc = spawn_httpd()
    try:
        req = requests.get(f"http://{HOST}:{PORT}/")
        assert req.status_code == 200
        assert req.text == "default page"
    finally:
        kill_httpd(proc)

def cleanup_daemon():
    if os.path.exists("/tmp/httpd_test.pid"):
        with open("/tmp/httpd_test.pid", "r") as f:
            pid = int(f.read().strip())
        try:
            os.kill(pid, 9)
        except:
            pass
        os.remove("/tmp/httpd_test.pid")

def test_daemon_start():
    cleanup_daemon()

    sp.run([EXECUTABLE, "--daemon", "start", "--pid_file", "/tmp/httpd_test.pid",
            "--ip", HOST, "--port", PORT, "--root_dir", ROOT_DIR,
            "--server_name", "test"])
    time.sleep(1)

    assert os.path.exists("/tmp/httpd_test.pid")

    with open("/tmp/httpd_test.pid", "r") as f:
        pid = int(f.read().strip())

    try:
        os.kill(pid, 0)
        running = True
    except:
        running = False

    assert running
    cleanup_daemon()

def test_daemon_stop():
    cleanup_daemon()
    sp.run([EXECUTABLE, "--daemon", "start", "--pid_file", "/tmp/httpd_test.pid",
            "--ip", HOST, "--port", PORT, "--root_dir", ROOT_DIR,
            "--server_name", "test"])
    time.sleep(1)

    with open("/tmp/httpd_test.pid", "r") as f:
        pid = int(f.read().strip())

    sp.run([EXECUTABLE, "--daemon", "stop", "--pid_file", "/tmp/httpd_test.pid",
            "--ip", HOST, "--port", PORT, "--root_dir", ROOT_DIR,
            "--server_name", "test"])
    time.sleep(1)

    try:
        os.kill(pid, 0)
        running = True
    except:
        running = False

    assert not running
    assert not os.path.exists("/tmp/httpd_test.pid")

def test_daemon_restart():
    cleanup_daemon()
    sp.run([EXECUTABLE, "--daemon", "start", "--pid_file", "/tmp/httpd_test.pid",
            "--ip", HOST, "--port", PORT, "--root_dir", ROOT_DIR,
            "--server_name", "test"])
    time.sleep(1)

    with open("/tmp/httpd_test.pid", "r") as f:
        old_pid = int(f.read().strip())

    sp.run([EXECUTABLE, "--daemon", "restart", "--pid_file", "/tmp/httpd_test.pid",
            "--ip", HOST, "--port", PORT, "--root_dir", ROOT_DIR,
            "--server_name", "test"])
    time.sleep(1)

    with open("/tmp/httpd_test.pid", "r") as f:
        new_pid = int(f.read().strip())

    assert old_pid != new_pid

    try:
        os.kill(old_pid, 0)
        old_running = True
    except:
        old_running = False

    try:
        os.kill(new_pid, 0)
        new_running = True
    except:
        new_running = False

    assert not old_running
    assert new_running
    cleanup_daemon()
