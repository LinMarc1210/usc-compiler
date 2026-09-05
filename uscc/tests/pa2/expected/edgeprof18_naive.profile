=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 100000000
while.cond -> while.end : 1
while.body -> if.then : 100000000
while.body -> if.else : 0
if.then -> if.end : 100000000
if.end -> if.then5 : 0
if.end -> if.else7 : 100000000
if.else -> if.end : 0
if.then5 -> if.end6 : 0
if.end6 -> if.then13 : 0
if.end6 -> if.else15 : 100000000
if.else7 -> if.end6 : 100000000
if.then13 -> if.end14 : 0
if.end14 -> if.then21 : 0
if.end14 -> if.else23 : 100000000
if.else15 -> if.end14 : 100000000
if.then21 -> if.end22 : 0
if.end22 -> if.then26 : 379505
if.end22 -> if.end27 : 99620495
if.else23 -> if.end22 : 100000000
if.then26 -> if.end27 : 379505
if.end27 -> while.cond : 100000000
END_PROFILE
