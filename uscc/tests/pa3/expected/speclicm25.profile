=== NAIVE EDGE PROFILING ===
EDGE_PROFILE: main
EDGES:
entry -> for.cond : 1
for.cond -> for.body : 30
for.cond -> for.end : 1
for.body -> if.then : 2
for.body -> if.end : 28
for.step -> for.cond : 30
for.end -> for.cond8 : 1
if.then -> if.end : 2
if.end -> for.step : 30
for.cond8 -> for.body9 : 10
for.cond8 -> for.end11 : 1
for.body9 -> for.step10 : 10
for.step10 -> for.cond8 : 10
END_PROFILE
