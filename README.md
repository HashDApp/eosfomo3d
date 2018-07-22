# eosfomo3d

这是一款在EOS上实现的FOMO 3D游戏  
如有错误之处，请多多指教，谢谢  

# EOS版FOMO 3D

# 部署
## 1、编译
```
make
```
## 2、设置合约
```
cleos set contract contractfomo ../eosfomo3d -p contractfomo@active
```
## 3、合约初始化
设置一些基本参数，如分组，分红比例，空投比例等  
```
cleos push action contractfomo contractinit '' -p contractfomo@active
```

# 开始游戏
## 1、充值
向合约账号转账EOS，作为购买key的资金  
memo字段需要填写为deposit  
```
cleos push action eosio.token transfer '[ "user", "contractfomo", "1.0000 EOS", "deposit" ]' -p user@active
```
## 2、购买key
4个参数含义如下:  
"user": 购买者账号名称  
0: 组id;0-Snek,1-Whale,2-Bull,3-Bear  
1: key数量  
0: 购买key使用的资金来源;0-充值资金,1-分红资金 
```
cleos push action contractfomo buykey '["user", 0, 1, 0]' -p user@active
```

## 3、提款
未购买key的余额可以随时提出  
```
cleos push action contractfomo withdraw '["user", "0.1000 EOS"]' -p user@active
```

# 游戏规则
1、最后一名购买key的玩家，在倒计时结束时，会获取所在组奖池中的50%作为奖励  
2、有玩家购买key后，倒计时重新开始  
3、  
