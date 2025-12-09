import subprocess
import time
import requests
import os
import sys

def test_vhosts():
    # Create dummy content
    os.makedirs("tests/www1", exist_ok=True)
    os.makedirs("tests/www2", exist_ok=True)
    with open("tests/www1/index.html", "w") as f:
        f.write("Hello from VHost 1")
    with open("tests/www2/index.html", "w") as f:
        f.write("Hello from VHost 2")

    # Start server
    print("Starting server...")
    server = subprocess.Popen(["./httpd", "-a", "start", "tests/test_vhosts.conf"])
    time.sleep(2) # Give it time to start

    try:
        # Test VHost 1
        print("Testing VHost 1 (Port 8080)...")
        r1 = requests.get("http://127.0.0.1:8080/index.html", timeout=2)
        print(f"VHost 1 Response: {r1.text}")
        assert "Hello from VHost 1" in r1.text
        assert r1.status_code == 200

        # Test VHost 2
        print("Testing VHost 2 (Port 8081)...")
        r2 = requests.get("http://127.0.0.1:8081/index.html", timeout=2)
        print(f"VHost 2 Response: {r2.text}")
        assert "Hello from VHost 2" in r2.text
        assert r2.status_code == 200

        print("SUCCESS: Both VHosts responded correctly.")

    except Exception as e:
        print(f"FAILURE: {e}")
        sys.exit(1)
    finally:
        print("Stopping server...")
        # We need to read the PID file to kill it properly, or just kill the process we started if it didn't daemonize fully yet (but it likely did)
        # The subject says "start" command starts the daemon.
        # Let's try to stop it using the stop command
        subprocess.run(["./httpd", "-a", "stop", "tests/test_vhosts.conf"])
        server.terminate()

import socket

def test_concurrency():
    print("\nTesting Concurrency...")
    # Start server
    print("Starting server for concurrency test...")
    server = subprocess.Popen(["./httpd", "-a", "start", "tests/test_vhosts.conf"])
    time.sleep(2)

    try:
        # 1. Open a "hanging" connection to VHost 1
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(("127.0.0.1", 8080))
        s.send(b"GET /index.html HTTP/1.1\r\nHost: 127.0.0.1\r\n") # Incomplete request
        print("  [1] Opened hanging connection to VHost 1 (sent partial headers)")

        # 2. Try to access VHost 2 immediately
        print("  [2] Attempting to access VHost 2...")
        start_time = time.time()
        r = requests.get("http://127.0.0.1:8081/index.html", timeout=2)
        duration = time.time() - start_time
        
        print(f"  [3] VHost 2 responded in {duration:.2f}s")
        assert r.status_code == 200
        assert "Hello from VHost 2" in r.text
        
        print("SUCCESS: Concurrency check passed (VHost 2 responded while VHost 1 was busy).")
        s.close()
        
    except Exception as e:
        print(f"FAILURE in concurrency test: {e}")
        sys.exit(1)
    finally:
        print("Stopping server...")
        subprocess.run(["./httpd", "-a", "stop", "tests/test_vhosts.conf"])
        server.terminate()

if __name__ == "__main__":
    test_vhosts()
    test_concurrency()
