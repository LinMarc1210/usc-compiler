=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 10
for.cond -> for.end : 1
for.body -> for.cond1 : 10
for.step -> for.cond : 10
for.cond1 -> for.body2 : 100
for.cond1 -> for.end4 : 10
for.body2 -> for.step3 : 100
for.step3 -> for.cond1 : 100
for.end4 -> if.then : 2
for.end4 -> if.end : 8
if.then -> if.end : 2
if.end -> for.step : 10
END_PROFILE
