=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 100
for.cond -> for.end : 1
for.body -> if.then : 3
for.body -> if.end : 97
for.step -> for.cond : 100
if.then -> if.end : 3
if.end -> for.step : 100
END_PROFILE
