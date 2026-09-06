=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 6
for.cond -> for.end : 1
for.body -> for.cond1 : 6
for.step -> for.cond : 6
for.cond1 -> for.body2 : 90
for.cond1 -> for.end4 : 6
for.body2 -> if.then : 18
for.body2 -> if.end : 72
for.step3 -> for.cond1 : 90
for.end4 -> for.step : 6
if.then -> if.end : 18
if.end -> for.step3 : 90
END_PROFILE
