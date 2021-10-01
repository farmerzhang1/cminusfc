# 实验说明

请 fork 此 repo 到自己的仓库下，随后在自己的仓库中完成实验，请确保自己的 repo 为 Private。

## 目前已布置的实验

* [lab1](./Documentations/1-parser/)
  + DDL：2021-10-03 23:59:59 (UTC+8)

## FAQ: How to merge upstream remote branches

In brief, you need another alias for upstream repository (we assume you are now in your local copy of forked repository on Gitlab):

```shell
$ git remote add upstream http://http://211.86.152.198//staff/2021fall-compiler_cminus.git
```

Then try to merge remote commits to your local repository:

```shell
$ git pull upstream master
```

Then synchronize changes to your forked remote repository:

```shell
$ git push origin master
```
