# EWBIK

## Requirement

1. install scoop

```powershell
> Set-ExecutionPolicy RemoteSigned -Scope CurrentUser # Optional: Needed to run a remote script the first time
> irm get.scoop.sh | iex
```

2. install scoop try 2

```powershell
scoop install git
scoop update
scoop bucket add games
scoop bucket add extras
scoop install sudo
```

3. `scoop install mingw`
1. install llvm-mingw with the following steps.
1. `scoop bucket add dorado https://github.com/chawyehsu/dorado`
1. `scoop install llvm-mingw`
1. git clone godot
1. git clone ewbik
1. put ewbik beside godot
1. scons custom_modules=../ewbik
