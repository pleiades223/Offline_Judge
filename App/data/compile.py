import subprocess
import time
import json

task = subprocess.Popen('cd data/judge && "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars64.bat" && cl /EHsc /O2 /std:c++20 /I../lib /O2 /DONLINE_JUDGE /DATCODER /FeOFFLINE_JUDGE_TEMP main.cpp',
                        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
start_time = time.time()
while task.poll() == None:
    if time.time() - start_time > 60:
        task.kill()
        break
    time.sleep(0.1)
with open('data/judge/judge.json', 'r', encoding='utf-8_sig') as js:
    judge = json.load(js)
judge["compile"] = str(task.returncode)
with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
    json.dump(judge, js, indent=4)
out = open('data/judge/compiler_out.txt', 'w', encoding='utf-8_sig')
stdout_data, stderr_data = task.communicate()
out.write(str(stdout_data, 'cp932'))
out.close()