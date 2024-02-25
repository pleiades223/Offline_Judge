import subprocess
import time
import json
import sys

with open('data/judge/judge.json', 'r', encoding="utf-8_sig") as js:
    judge = json.load(js)
timelimit = float(judge["TL"])
task_path = judge["task-input"]
with open(task_path)as input:
    task = subprocess.Popen("call data\judge\OFFLINE_JUDGE_TEMP.exe", shell=True, stdin=input,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
start_time = time.time()
while task.poll() == None:
    if time.time()-start_time > timelimit:
        subprocess.run("taskkill /F /IM OFFLINE_JUDGE_TEMP.exe")
        judge["result"] = "TLE"
        judge["runtime"] = str(time.time()-start_time)
        with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
            json.dump(judge, js, indent=4)
        out = open('data/judge/out.txt', 'w', encoding='utf-8_sig')
        sys.exit()
    time.sleep(0.0001)
if task.returncode == 0:
    judge["result"] = "JUDGE"
else:
    judge["result"] = "RE"
judge["runtime"] = str(time.time()-start_time)
with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
    json.dump(judge, js, indent=4)
out = open('data/judge/out.txt', 'w', encoding='utf-8_sig')
stdout_data, stderr_data = task.communicate()
out.write(str(stdout_data, 'cp932'))