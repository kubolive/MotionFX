import subprocess
import time
import os

OBS_PATH = r"C:\Program Files\obs-studio\bin\64bit\obs64.exe"
OBS_DIR = r"C:\Program Files\obs-studio\bin\64bit"

DETACHED_PROCESS = 0x00000008
CREATE_NEW_PROCESS_GROUP = 0x00000200

# Launch OBS detached so it continues running after this script terminates
proc = subprocess.Popen(
    [OBS_PATH, "--disable-shutdown-check"],
    cwd=OBS_DIR,
    creationflags=DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
    close_fds=True
)

print(f"Launched OBS Studio persistently with PID {proc.pid}")
