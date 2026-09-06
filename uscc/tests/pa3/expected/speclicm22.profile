=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 5
for.cond -> for.end : 1
for.body -> if.then : 2
for.body -> if.end : 3
for.step -> for.cond : 5
if.then -> if.end : 2
if.end -> for.step : 5
END_PROFILE
