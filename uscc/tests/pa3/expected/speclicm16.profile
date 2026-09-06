=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 64
for.cond -> for.end : 1
for.body -> if.then : 2
for.body -> if.end : 62
for.step -> for.cond : 64
if.then -> if.end : 2
if.end -> for.step : 64
END_PROFILE
