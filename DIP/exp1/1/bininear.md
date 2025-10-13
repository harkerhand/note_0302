假设有一个二维网格，四个点的值已知：

```
f11 ---- f12
 |        |
 |        |
f21 ---- f22
```

* 左上角 $f11 = f(x_1, y_1)$
* 右上角 $f12 = f(x_2, y_1)$
* 左下角 $f21 = f(x_1, y_2)$
* 右下角 $f22 = f(x_2, y_2)$

我们想求网格内某一点 $(x, y)$ 的值 $f(x, y)$

定义局部坐标：

$$
dx = \frac{x - x_1}{x_2 - x_1}, \quad dy = \frac{y - y_1}{y_2 - y_1}
$$

* $dx, dy \in [0,1]$
* $dx = 0$ 在左边，$dx = 1$ 在右边
* $dy = 0$ 在上边，$dy = 1$ 在下边


**步骤 1：在 x 方向上做线性插值**

在 y 固定为 y1 的行：

$$
f(x, y_1) = f11 \cdot (1 - dx) + f12 \cdot dx
$$

在 y 固定为 y2 的行：

$$
f(x, y_2) = f21 \cdot (1 - dx) + f22 \cdot dx
$$

---

**步骤 2：在 y 方向上做线性插值**

用上一步得到的两个结果，在 y 方向上插值：

$$
f(x, y) = f(x, y_1) \cdot (1 - dy) + f(x, y_2) \cdot dy
$$


把步骤 1 的表达式代入步骤 2：

$$
\begin{aligned}
f(x, y) &= \big(f11 (1-dx) + f12 dx \big) (1-dy) + \big(f21 (1-dx) + f22 dx \big) dy \
&= f11 (1-dx)(1-dy) + f12 dx (1-dy) + f21 (1-dx) dy + f22 dx dy
\end{aligned}
$$

```cpp
double value = f11 * (1 - dx) * (1 - dy) +
               f12 * dx * (1 - dy) +
               f21 * (1 - dx) * dy +
               f22 * dx * dy;
```