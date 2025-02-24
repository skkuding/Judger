# Judger 

A sandbox to securely execute untrusted programs in the judging system

## How to build `libjudger.a`

1. Reopen in container  
2. Execute the following command in the terminal  

```bash
./build.sh
```

## What methodological changes have been made

For the original judger, the sandbox is implemented by `setrlimit`.  
However, the `setrlimit` method is not precise enough to limit the memory usage of the program.  
Therefore, we use `cgroup` to limit the memory usage of the program.  

[Document](https://opensource.qduoj.com#/judger/api)