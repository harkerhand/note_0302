# 第一次实验

在VLAN视图下清空MAC地址

```bash
undo mac-address
vlan 1
mac-address learning disable
display mac-address
```

![image-20251113191549895](./README/image-20251113191549895.png)

![image-20251113201338295](./README/image-20251113201338295.png)

![image-20251113201412927](./README/image-20251113201412927.png)

依然是空表

![image-20251113194034635](./README/image-20251113194034635.png)

打开学习后ping，第一次没有reply，第二次没收到

![6114c801bb4885ffa5b2754a2d240ad3](./README/6114c801bb4885ffa5b2754a2d240ad3.png)

同时表更新

![image-20251113201959769](./README/image-20251113201959769.png)

![image-20251113204452619](./README/image-20251113204452619.png)

作为Switch3设置vlan

![image-20251113204425309](./README/image-20251113204425309.png)

![image-20251113204839264](./README/image-20251113204839264.png)

![image-20251113205142316](./README/image-20251113205142316.png)