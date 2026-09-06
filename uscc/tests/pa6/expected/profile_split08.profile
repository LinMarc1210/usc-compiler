=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> while.cond : 1
while.cond -> while.body : 200
while.cond -> while.end : 1
while.body -> if.then : 50
while.body -> if.else : 150
if.then -> if.end : 50
if.end -> while.cond : 200
if.else -> if.then2 : 50
if.else -> if.else4 : 100
if.then2 -> if.end3 : 50
if.end3 -> if.end : 150
if.else4 -> if.then6 : 50
if.else4 -> if.else8 : 50
if.then6 -> if.end7 : 50
if.end7 -> if.end3 : 100
if.else8 -> if.end7 : 50
END_PROFILE
