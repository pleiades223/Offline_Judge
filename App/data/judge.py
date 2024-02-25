import json
import math
import sys
out = open('data/judge/out.txt','r', encoding='utf-8_sig').read().split()
judge = json.load(open('data/judge/judge.json', 'r', encoding='utf-8_sig'))
ans = open(judge["task-output"],'r', encoding='utf-8_sig').read().split()
if len(out)==len(ans):
    print(0)
    n=len(ans)
    for i in range(n):
        if ans[i]!=out[i]:
            if len(ans[i])>20:
                judge["result"]="WA"
                with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
                    json.dump(judge, js, indent=4)
                sys.exit()
            try:
                a = float(ans[i])
                try:
                    o = float(out[i])
                    if '.' in ans[i]:
                        if math.isclose(a,o,rel_tol=1e-6)==False:
                            judge["result"]="WA"
                            with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
                                json.dump(judge, js, indent=4)
                            sys.exit()
                    else:
                        judge["result"]="WA"
                        with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
                            json.dump(judge, js, indent=4)
                        sys.exit()
                except ValueError:
                    judge["result"]="WA"
                    with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
                        json.dump(judge, js, indent=4)
                    sys.exit()
            except ValueError:
                judge["result"]="WA"
                with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
                    json.dump(judge, js, indent=4)
                sys.exit()
    judge["result"]="AC"
    with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
        json.dump(judge, js, indent=4)
else:
    judge["result"]="WA"
    with open("data/judge/judge.json", "w", encoding='utf-8_sig') as js:
        json.dump(judge, js, indent=4)