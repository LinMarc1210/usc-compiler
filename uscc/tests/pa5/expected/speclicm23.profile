=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> if.then : 0
entry -> if.else : 1
if.then -> for.cond : 0
if.else -> if.end : 1
for.cond -> for.body : 0
for.cond -> for.end : 0
for.body -> if.then3 : 0
for.body -> if.end4 : 0
for.step -> for.cond : 0
for.end -> if.end : 0
if.then3 -> if.end4 : 0
if.end4 -> for.step : 0
END_PROFILE
