# NPC系统使用说明

## 概述
本NPC系统专为滑雪大冒险游戏设计，所有NPC都强制向右移动以简化游戏逻辑。

## 设计特点
- **强制向右移动**: 所有NPC只能向右移动，无法改变移动方向
- **类型区分**: 支持Ground（地面动物）和Flying（飞鸟）两种类型
- **易于继承**: 基类提供了完整的虚函数接口供子类扩展
- **工厂模式**: 提供统一的NPC创建接口

## 核心类结构

### NPCEntity (基类)
- `NPCType`: 枚举类型，区分Ground和Flying
- `setMovementSpeed()`: 设置移动速度
- `getTargetVelocityX()`: 返回固定的右向速度
- `updateSpecialAbility()`: 虚函数，供子类实现特殊能力

### NPCFactory (工厂类)
- `createGroundNPC()`: 创建地面动物NPC
- `createFlyingNPC()`: 创建飞鸟NPC
- `createNPC()`: 通用创建方法

## 基本使用

```cpp
// 创建地面动物
auto groundNPC = NPCFactory::createGroundNPC(QPointF(100, 200));

// 创建飞鸟
auto flyingNPC = NPCFactory::createFlyingNPC(QPointF(150, 100));

// 添加到场景
scene->addItem(groundNPC.get());
scene->addItem(flyingNPC.get());
```

## 扩展指南

### 创建自定义NPC子类
1. 继承NPCEntity
2. 重写虚函数：
   - `initializeNPC()`: 初始化
   - `updateSpecialAbility()`: 特殊能力
   - `updateAI()`: AI逻辑
   - `updateAppearance()`: 外观更新

### 示例子类
参考 `groundnpc.h` 和 `flyingnpc.h` 的实现方式。

## 重要说明
- 所有NPC的水平移动速度由`m_movementSpeed`控制
- NPC的移动方向已固定为向右，无法更改
- 飞鸟的特殊能力应在`updateSpecialAbility()`中实现
- 使用工厂模式创建NPC可确保正确的初始化配置
