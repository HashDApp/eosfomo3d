# datalog

本合约提供了一种方法  
可以将重要信息保存在链上  
通过检索链上数据进行追溯  

## 记录日志
日志字符串的格式可以自定义  
```
cleos push action datalog log '["datalog", "id,username,amount,notes"]' -p datalog@active
```

## 添加用户
```
cleos push action datalog adduser '["contractfomo"]' -p datalog@active
```

## 删除用户
```
cleos push action datalog rmuser '["contractfomo"]' -p datalog@active
```
