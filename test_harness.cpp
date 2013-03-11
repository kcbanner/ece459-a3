/* to generate the makefile, qmake -o Makefile test-harness.pro */

#include <iostream>

#include <QtGui/QApplication>
#include "model.h"
#include "window.h"

int main(int argc, char * argv[]) {
    QCoreApplication a(argc, argv);

    Model m;

    m.imgs[0]->load((argc > 2) ? argv[1] : "mountains.jpg");
    m.imgs[1]->load((argc > 2) ? argv[2] : "beaches.jpg");

	m.wimg = m.imgs[0]->width();
	m.himg = m.imgs[0]->height();

	m.listLines[0] = new vector<pair<QPoint, QPoint> >();
	m.listLines[1] = new vector<pair<QPoint, QPoint> >();

	m.listAux[0] = new vector<pair<QPoint, QPoint> >();
	m.listAux[1] = new vector<pair<QPoint, QPoint> >();

	m.imgs[2] = new QImage(m.wimg, m.himg, QImage::Format_RGB32);
	m.imgs[4] = new QImage(m.wimg, m.himg, QImage::Format_RGB32);

	QPoint pt1(0,0), pt2(2000,2000);
	pair<QPoint, QPoint> pp(pt1, pt2);

	QPoint pt3(4000,3000), pt4(2000,1000);
	pair<QPoint, QPoint> pp2(pt3, pt4);

	m.listLines[0]->push_back(pp);
	m.listLines[1]->push_back(pp2);

	m.prepStraightLine();
	m.commonPrep();
    m.morph(0, 0.0, 1.0, 0.5);

	m.imgs[2]->save("out.jpg");
	a.exit(0);
}
