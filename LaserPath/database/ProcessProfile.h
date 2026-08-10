#pragma once
#include <QString>


struct ProcessProfile
{
	int id = 0;
	QString name;
	QString createdAt;
	QString updatedAt;

	// 切片
	double layerHeight = 2.0;
	double firstLayerHeight = 2.0;
	bool autoRange = true;
	double zMin = 0.0;
	double zMax = 0.0;

	// 精度
	double minContourLen = 2.0;
	double collinearPrecision = 2.0;
	double gapCloseRadius = 2.0;

	// 偏移
	bool enableOffset = true;
	double outerWallWidth = 2.0;
	double innerWallWidth = 2.0;
	int baseContour = 0;          // 0=全轮廓, 1=外轮廓, 2=内轮廓
	bool autoOffset = false;
	int offsetCount = 0;
	double offsetContourComp = 2.0;
	double startAngle = 2.0;
	int startPointStrategy = 0;   // 0=固定起点, 1=按照角度, 2=按照长度
	double angleIncrement = 2.0;
	double lengthIncrement = 2.0;

	// 填充
	double beadWidth = 2.0;
	int fillType = 0;             // 0=直线, 1=往复
	bool adaptiveOverlap = false;
	double maxOverlapRate = 0.0;  // 自适应-最小搭接率
	double minOverlapRate = 0.0;  // 自适应-最大搭接率
	double overlapRate = 0.0;     // 固定搭接率
	int marginStrategy = 0;       // 0=自由, 1=居中
	int scanDirection = 0;        // 0=指定角度, 1=沿长边, 2=沿短边
	double scanAngle = 0.0;
	double interlayerRotation = 0.0;
	double fillAreaComp = 0.0;
	double endExtension = 0.0;
};
