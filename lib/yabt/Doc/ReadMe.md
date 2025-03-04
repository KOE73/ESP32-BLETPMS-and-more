cd lib/yabt/bluetooth-SIG

git submodule add https://bitbucket.org/bluetooth-SIG/public.git lib/yabt/bluetooth-SIG

git submodule update --remote

git commit -m "Submodule add bluetooth-SIG"

# Update

git submodule update --remote

```
git pull --recurse-submodules
git submodule update --remote
```

# On clone

git submodule update --init --recursive


# Remove

git submodule deinit -f lib/yabt/bluetooth-SIG
rm -rf lib/yabt/bluetooth-SIG
git rm --cached lib/yabt/bluetooth-SIG