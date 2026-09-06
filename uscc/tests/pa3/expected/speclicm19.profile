=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 16
for.cond -> for.end : 1
for.body -> if.then : 2
for.body -> if.end : 14
for.step -> for.cond : 16
if.then -> if.end : 2
if.end -> for.step : 16
END_PROFILE
