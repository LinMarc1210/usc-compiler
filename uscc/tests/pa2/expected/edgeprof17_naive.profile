=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 100000000
if.then -> if.end : 100000000
if.end -> if.then4 : 66666667
if.end -> if.else6 : 133333333
if.else -> if.end : 100000000
if.then4 -> if.end5 : 66666667
if.end5 -> if.then11 : 40000000
if.end5 -> if.else13 : 160000000
if.else6 -> if.end5 : 133333333
if.then11 -> if.end12 : 40000000
if.end12 -> if.then18 : 28571429
if.end12 -> if.else20 : 171428571
if.else13 -> if.end12 : 160000000
if.then18 -> if.end19 : 28571429
if.end19 -> if.then25 : 18181819
if.end19 -> if.else27 : 181818181
if.else20 -> if.end19 : 171428571
if.then25 -> if.end26 : 18181819
if.end26 -> while.cond : 200000000
if.else27 -> if.end26 : 181818181
END_PROFILE
