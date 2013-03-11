#ifndef MODEL_H
#define MODEL_H

#include <QtGui>
#include <QGraphicsView>

using namespace std;

class Model {
 public:
	Model();
	//~Model();
	
	void prepStraightLine();
	void commonPrep();
	void morph(int h, double VARA, double VARB, double VARP);

	int himg, wimg;
    vector<pair<QPoint, QPoint> > **listLines;
    vector<pair<QPoint, QPoint> > **listAux;
    QImage *imgs[5];
};
#endif
