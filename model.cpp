#include "model.h"

typedef double  v4df  __attribute__ ((vector_size (32)));  /* double[4], AVX  */

union vec4d {
	v4df v;
	double a[4];
};

Model::Model() {
  for (unsigned int i = 0; i < NUM_IMAGES; ++i) imgs[i] = new QImage();

  listLines = new vector<pair <QPoint, QPoint> >* [2];
  listAux = new vector<pair <QPoint, QPoint> >* [2];
}

void Model::prepStraightLine() {
  QRgb red = qRgb(255, 0, 0);
  QRgb blue = qRgb(0, 0, 255);

  for(unsigned int k=0; k<listLines[0]->size(); ++k) {
    double x3, x4, y3, y4;
    x3 = x4 = 0;
    y3 = y4 = 0;

    for(int h=0; h<2; ++h) {
      double x1 = listLines[h]->at(k).first.x();
      double y1 = listLines[h]->at(k).first.y();

      double x2 = listLines[h]->at(k).second.x();
      double y2 = listLines[h]->at(k).second.y();

      double m = (y2 - y1) / (x2 - x1);

      x3 += x1 / 2;
      x4 += x2 / 2;

      y3 += y1 / 2;
      y4 += y2 / 2;

      // if x's are further apart than 'y's
      if(fabs(x1-x2) > fabs(y1-y2)) {
        // swap coordinates if backwards
        if(x1 > x2) { swap(x1, x2); swap(y1, y2); }

        // draw line, with the appropriate slope
        while(x1 < x2) {
          y1 = m*(x1 - x2) + y2;
          imgs[4]->setPixel(x1, y1, h?red:blue);
          x1 += 0.01;
        }
      } else {
        // swap coordinates if backwards
        if(y1 > y2) { swap(x1, x2); swap(y1, y2); }

        // draw line, with the appropriate slope
        while(y1 < y2) {
          x1 = (y1 - y2)/m + x2;
          imgs[4]->setPixel(x1, y1, h?red:blue);
          y1 += 0.01;
        }
      }
    }           

    pair<QPoint, QPoint> p = make_pair(QPoint(x3, y3), QPoint(x4, y4));
    listAux[0]->push_back(p);
    listAux[1]->push_back(p);
  }
}

void Model::commonPrep() {
  for(unsigned int k=0; k<listAux[0]->size(); ++k) {
    double x3, x4, y3, y4;

    x3 = listAux[0]->at(k).first.x();
    y3 = listAux[0]->at(k).first.y();

    x4 = listAux[0]->at(k).second.x();
    y4 = listAux[0]->at(k).second.y();

    double m = (y4 - y3) / (x4 - x3);
    // if x's are further apart than 'y's
    if(fabs(x4-x3) > fabs(y4-y3)) {
      // swap coordinates if backwards
      if(x3 > x4) { swap(x3, x4); swap(y3, y4); }

      // draw line, with the appropriate slope
      while(x3 < x4) {
        y3 = m*(x3 - x4) + y4;
        imgs[4]->setPixel(x3, y3, qRgb(0, 0, 0));
        x3 += 0.01;
      }
    } else {
      // swap coordinates if backwards
      if(y3 > y4) { swap(x3, x4); swap(y3, y4); }

      // draw line, with the appropriate slope
      while(y3 < y4) {
        x3 = (y3 - y4)/m + x4;
        imgs[4]->setPixel(x3, y3, qRgb(0, 0, 0));
        y3 += 0.01;
      }
    }
  }
}

void Model::morph(int h, double VARA, double VARB, double VARP) {
  int n = 0;
  int lines = listLines[0]->size();

  #pragma omp parallel for
  for(int i=0; i<wimg; ++i) {
    for(int j=0; j<himg; ++j) {
            
      QPoint X(i, j);
      double u, v;
            
      vec4d ww;
      //QPoint pp[lines];
      vec4d pp_x;
      vec4d pp_y;	
            
      // for each line
      for(int k=0; k<lines; ++k) {
                
        // get original lines from reference line
        QPoint P = listLines[h]->at(k).first;
        QPoint Q = listLines[h]->at(k).second;
                
        QVector2D XP(X - P);
        QVector2D QP(Q - P);
                
        QVector2D pQP(QP.y(), -QP.x());
                
        // Calculate u, v
        u = QVector2D::dotProduct(XP, QP) /  QP.lengthSquared();
        v = QVector2D::dotProduct(XP, pQP) / QP.length();

        // get interpolating lines from reference line
        QPoint P2 = listAux[h]->at(k).first;
        QPoint Q2 = listAux[h]->at(k).second;

        QVector2D Q2P2(Q2 - P2);
        QVector2D pQ2P2(Q2P2.y(), -Q2P2.x());

        QVector2D X2 = QVector2D(P2) + u * Q2P2 + (v * pQ2P2) / Q2P2.length();

        QPoint p = X2.toPoint() - X;

        double dist = 0;
        if(u > 0 && u < 1) dist = fabs(v);
        else if(u <= 0) dist = sqrt(pow(X.x() - P.x(), 2.0) + pow(X.y() - P.y(), 2.0));
        else dist = sqrt(pow(X.x() - Q.x(), 2.0) + pow(X.y() - Q.y(), 2.0));

        double w = 0;
        w =  pow(QP.length(), VARP);
        w /= (VARA + dist);
        w = pow(w, VARB);

        ww.a[k] = w;
        pp_x.a[k] = p.x();
		pp_y.a[k] = p.y();
      }

      //QPoint sum(0.0, 0.0);
      int sum_x = 0;
      int sum_y = 0;
      double wsum = 0.0;
      //printf("lines: %d\n", lines);
	    
      if (lines == 4) {
        vec4d product;
	       vec4d sum;

	       product.v = __builtin_ia32_mulpd256(ww.v, pp_x.v);
	       sum.v = __builtin_ia32_roundpd256(product.v, 0);
	
	sum_x = (int)sum.a[0] + (int)sum.a[1] + (int)sum.a[2] + (int)sum.a[3];
	
	
	product.v = __builtin_ia32_mulpd256(ww.v, pp_y.v);
	sum.v = __builtin_ia32_roundpd256(product.v, 0);	
	 
	sum_y = (int)sum.a[0] + (int)sum.a[1] + (int)sum.a[2] + (int)sum.a[3];


        wsum += ww.a[0];
        wsum += ww.a[1];
        wsum += ww.a[2];
        wsum += ww.a[3];
      } else {

        for(int k=0; k<lines; ++k) {
          sum_x  += ww.a[k] * pp_x.a[k];
          sum_y  += ww.a[k] * pp_y.a[k];			
          wsum += ww.a[k];
        }
      }

      sum_x = qRound(sum_x / wsum);
      sum_y = qRound(sum_y / wsum);

      QPoint X2 = X + QPoint(sum_x, sum_y);

      double y0, x0;
      x0 = ceil(X2.x());
      if(x0 < 0) x0 = 0;
      if(x0 >= wimg) x0 = wimg-1;

      y0 = ceil(X2.y());
      if(y0 < 0) y0 = 0;
      if(y0 >= himg) y0 = himg-1;

      X2 = QPoint(x0, y0);

      if(X2 == X) n++;
      imgs[h+2]->setPixel(X, imgs[h]->pixel(X2));
    }
  }
}
