#!/bin/sh
# echo the current version
echo "$(git show-ref --tags | grep $(git show-ref --tags latest | awk '{print $1;}') | grep -v "latest" | cut -d "/" -f3).$(git rev-parse --abbrev-ref HEAD)#$(git rev-list $(git show-ref --tags | grep $(git show-ref --tags latest | awk '{print $1;}') | grep -v "latest" | cut -d "/" -f3)..HEAD --count)"