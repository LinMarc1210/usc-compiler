=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.end : 100000000
if.then -> if.end : 100000000
if.end -> if.then3 : 66666667
if.end -> if.end4 : 133333333
if.then3 -> if.end4 : 66666667
if.end4 -> if.then8 : 40000000
if.end4 -> if.end9 : 160000000
if.then8 -> if.end9 : 40000000
if.end9 -> if.then13 : 28571429
if.end9 -> if.end14 : 171428571
if.then13 -> if.end14 : 28571429
if.end14 -> if.then18 : 18181819
if.end14 -> if.end19 : 181818181
if.then18 -> if.end19 : 18181819
if.end19 -> while.cond : 200000000
END_PROFILE
