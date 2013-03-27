#include <pthread.h>
#include "model.h"

#define NUM_THREADS 16

typedef double  v4df  __attribute__ ((vector_size (32)));  /* double[4], AVX  */

union vec4d {
	v4df v;
	double a[4];
};

struct MemoizedData {
  int h;
  double VARA;
  double VARB;
  double VARP;

  pair<QPoint, QPoint>* linesData;
  pair<QPoint, QPoint>* auxData;
  QImage** imgs;
  int lines;
  int himg;
  int wimg;
  double *QPs_x;
  double *QPs_y;
  double *pQPs_x;
  double *pQPs_y;
  QVector2D *Q2P2s;
  QVector2D *pQ2P2s;
  double *QPlengths;
  double *QPlengthsSquared;
  double *Q2P2lengths;
  double *powVARP;
};

struct ThreadData {
  int i;
  int iterations;
  
  MemoizedData* data;
};

void* morph_thread(void* arg);

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
  int lines = listLines[0]->size();
  
  pair<QPoint, QPoint>* linesData = listLines[h]->data();
  pair<QPoint, QPoint>* auxData = listAux[h]->data();

  // Memoize  

  double QPs_x[lines];
  double QPs_y[lines];  
  double pQPs_x[lines];
  double pQPs_y[lines];
  QVector2D Q2P2s[lines];
  QVector2D pQ2P2s[lines];
  double QPlengths[lines];
  double QPlengthsSquared[lines];
  double Q2P2lengths[lines];
  double powVARP[lines];
  for(int k = 0; k < lines; ++k) {
    QVector2D QP;
    QVector2D pQP;
    
    QPoint P = linesData[k].first;
    QPoint Q = linesData[k].second;
    QPoint P2 = auxData[k].first;
    QPoint Q2 = auxData[k].second;

    QP = QVector2D(Q - P);
    pQP = QVector2D(QP.y(), -QP.x());
    QPlengthsSquared[k] = QP.lengthSquared();
    QPlengths[k] = QP.length();
    
    QPs_x[k] = QP.x();
    QPs_y[k] = QP.y();
    pQPs_x[k] = pQP.x();
    pQPs_y[k] = pQP.y();

    powVARP[k] = pow(QPlengths[k], VARP);
    Q2P2s[k] = QVector2D(Q2 - P2);
    pQ2P2s[k] = QVector2D(Q2P2s[k].y(), -Q2P2s[k].x());
    Q2P2lengths[k] = Q2P2s[k].length();
  }


  MemoizedData data;
  data.wimg = wimg;
  data.himg = himg;
  data.imgs = imgs;
  data.linesData = linesData;
  data.auxData = auxData;
  data.h = h;
  data.lines = lines;
  data.VARA = VARA;
  data.VARB = VARB;
  data.VARP = VARP;
  data.QPs_x = QPs_x;
  data.QPs_y = QPs_y;
  data.pQPs_x = pQPs_x;
  data.pQPs_y = pQPs_y;
  data.Q2P2s = Q2P2s ;
  data.pQ2P2s = pQ2P2s;
  data.QPlengths = QPlengths;
  data.QPlengthsSquared = QPlengthsSquared;
  data.Q2P2lengths = Q2P2lengths;
  data.powVARP = powVARP;

  
  pthread_t threads[NUM_THREADS];
  ThreadData threadData[NUM_THREADS];

  int offset_stride = himg/NUM_THREADS;
  int offset = 0;
  for(int i=0; i <NUM_THREADS; i++) {
    threadData[i].i = offset;
    threadData[i].iterations = offset_stride;
    threadData[i].data = &data;
    offset += offset_stride;

    if(i == NUM_THREADS -1) {
      threadData[i].iterations += himg % NUM_THREADS;
    }
    
    pthread_create(&threads[i], NULL,  morph_thread, &threadData[i]);
  }

  for(int i=0; i <NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  

}

void* morph_thread(void* arg) {
  ThreadData* threadData = (ThreadData*) arg;
  MemoizedData* d = threadData->data;

  int h = d->h;
  int lines = d->lines;
  double VARA = d->VARA;
  double VARB = d->VARB;
  QImage** imgs = d->imgs;
  int wimg = d->wimg;
  int himg = d->himg;

  for(int j=threadData->i; j< (threadData->i + threadData->iterations); ++j) {
    for(int i=0; i<wimg; ++i) {
      QPoint X(i, j);
            
      vec4d ww;
      vec4d pp_x;
      vec4d pp_y;
      
      double u[lines];
      double v[lines];

      for(int k = 0; k < lines; ++k) {
        QPoint P = d->linesData[k].first;
        double XPx = X.x() - P.x();
        double XPy = X.y() - P.y();

        u[k] = (XPx * d->QPs_x[k] + XPy * d->QPs_y[k]) / d->QPlengthsSquared[k];
        v[k] = (XPx * d->pQPs_x[k] + XPy * d->pQPs_y[k]) / d->QPlengths[k];        
      }

      QVector2D X2s[lines];
      for(int k = 0; k < lines; ++k) {
        QPoint P2 = d->auxData[k].first;
        X2s[k] = QVector2D(P2) + u[k] * d->Q2P2s[k] + (v[k] * d->pQ2P2s[k]) / d->Q2P2lengths[k];
      }

      for(int k = 0; k < lines; ++k) {
        QPoint P = d->linesData[k].first;

        double dist = 0;
        if(u[k] > 0 && u[k] < 1) {
          dist = fabs(v[k]);
        } else {
          QPoint Q = d->linesData[k].second;

          if(u[k] <= 0) {
            dist = sqrt(pow(X.x() - P.x(), 2.0) + pow(X.y() - P.y(), 2.0));
          } else {
            dist = sqrt(pow(X.x() - Q.x(), 2.0) + pow(X.y() - Q.y(), 2.0));
          }
        }

        double w;
        w =  d->powVARP[k];
        w /= (VARA + dist);
        w = pow(w, VARB);

        ww.a[k] = w;
        pp_x.a[k] = qRound(X2s[k].x()) - X.x();
        pp_y.a[k] = qRound(X2s[k].y()) - X.y();
      }

      int sum_x = 0;
      int sum_y = 0;
      double wsum = 0.0;
      
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
        printf("error\n");

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

      imgs[h+2]->setPixel(X, imgs[h]->pixel(X2));
    }
  }
  return NULL;
}
