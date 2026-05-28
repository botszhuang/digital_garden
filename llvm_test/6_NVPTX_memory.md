---
title: "NVPTX Memory Hierarchy"
author: "Botsz"
date: 2026-05-29
---
## NVPTX Memory Hierarchy
The following table details the relationship between LLVM `addrspace(N)` attributes, CUDA memory spaces, and their physical performance characteristics:

✅ NVIDIA GPU devices (generally) have four types of memory
|| LLVM IR Syntax | CUDA Memory Space | Hardware Speed / Type | Hardware Scope & Description |
| :---|:---: | :--- | :--- | :--- |
| |**0** | Generic | Dynamic / Varies | The default address space |
| ✅ |**1** | Global | Slow  | Large, off-chip memory |
| |**2** | Internal Use | — | |
| ✅ |**3** | Shared | Fast  | on-chip memory shared among all threads in a CTA |
| ✅ |**4** | Constant | Fast | Read-only memory shared across all threads |
| ✅ |**5** | Local | Slow  | Per-thread, private memory |
| |**6** | |  |  |
| |**7** | Shared Cluster | Extremely Fast (Distributed) | |
---

# Reference
[1] User Guide for NVPTX Back-end https://llvm.org/docs/NVPTXUsage.html