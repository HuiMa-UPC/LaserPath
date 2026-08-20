#pragma once
#include <QString>
#include <QByteArray>


struct MachineProfile
{
    int id = 0;
    QString name;
    QString createdAt;
    QString updatedAt;

    // 基础信息
    int kinematics = 0;         // 0=三轴CNC, 1=五轴CNC, 2=六轴KUKA, 3=八轴KUKA
    int heatSource = 0;         // 0=激光, 1=电弧
    double maxHeight = 500.0;
    double maxX = 500.0;
    double maxY = 500.0;
    double originX = 0.0;
    double originY = 0.0;
    QString imagePath;
    QByteArray imageData;

    // G-code
    QString printStartGcode;    // 打印起始
    QString printEndGcode;      // 打印结束
    QString pathStartGcode;     // 轨迹起始
    QString pathEndGcode;       // 轨迹结束
    QString layerStartGcode;    // 层开始
    QString layerEndGcode;      // 层结束
    QString interlayerCoolingGcode; // 特殊层间冷却

    // 运动能力
    double maxSpeedX = 1000.0;
    double maxSpeedY = 1000.0;
    double maxSpeedZ = 1000.0;
    double maxSpeedE = 1000.0;

    // 备注
    QString notes;
};
