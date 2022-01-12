#include "number.h"

#include <TJpg_Decoder.h>
//int numx;
//int numy;
//int numn;

//显示白色36*60大小数字
void Number::printfW3660(int numx,int numy,int numn)
{
  switch(numn)
  {
    case 0:
      TJpgDec.drawJpg(numx,numy,W_3660_i0, sizeof(W_3660_i0));
      break;
    case 1:
      TJpgDec.drawJpg(numx,numy,W_3660_i1, sizeof(W_3660_i1));
      break;
    case 2:
      TJpgDec.drawJpg(numx,numy,W_3660_i2, sizeof(W_3660_i2));
      break;
    case 3:
      TJpgDec.drawJpg(numx,numy,W_3660_i3, sizeof(W_3660_i3));
      break;
    case 4:
      TJpgDec.drawJpg(numx,numy,W_3660_i4, sizeof(W_3660_i4));
      break;
    case 5:
      TJpgDec.drawJpg(numx,numy,W_3660_i5, sizeof(W_3660_i5));
      break;
    case 6:
      TJpgDec.drawJpg(numx,numy,W_3660_i6, sizeof(W_3660_i6));
      break;
    case 7:
      TJpgDec.drawJpg(numx,numy,W_3660_i7, sizeof(W_3660_i7));
      break;
    case 8:
      TJpgDec.drawJpg(numx,numy,W_3660_i8, sizeof(W_3660_i8));
      break;
    case 9:
      TJpgDec.drawJpg(numx,numy,W_3660_i9, sizeof(W_3660_i9));
      break;
    default:
      Serial.println("显示W_3660数字错误");
      break;
  }
}
//显示橙色36*60大小数字
void Number::printfO3660(int numx,int numy,int numn)
{
  switch(numn)
  {
    case 0:
      TJpgDec.drawJpg(numx,numy,O_3660_i0, sizeof(O_3660_i0));
      break;
    case 1:
      TJpgDec.drawJpg(numx,numy,O_3660_i1, sizeof(O_3660_i1));
      break;
    case 2:
      TJpgDec.drawJpg(numx,numy,O_3660_i2, sizeof(O_3660_i2));
      break;
    case 3:
      TJpgDec.drawJpg(numx,numy,O_3660_i3, sizeof(O_3660_i3));
      break;
    case 4:
      TJpgDec.drawJpg(numx,numy,O_3660_i4, sizeof(O_3660_i4));
      break;
    case 5:
      TJpgDec.drawJpg(numx,numy,O_3660_i5, sizeof(O_3660_i5));
      break;
    case 6:
      TJpgDec.drawJpg(numx,numy,O_3660_i6, sizeof(O_3660_i6));
      break;
    case 7:
      TJpgDec.drawJpg(numx,numy,O_3660_i7, sizeof(O_3660_i7));
      break;
    case 8:
      TJpgDec.drawJpg(numx,numy,O_3660_i8, sizeof(O_3660_i8));
      break;
    case 9:
      TJpgDec.drawJpg(numx,numy,O_3660_i9, sizeof(O_3660_i9));
      break;
    default:
      Serial.println("显示O_3660数字错误");
      break;
  }
}
//显示白色18*30大小数字
void Number::printfW1830(int numx,int numy,int numn)
{
  switch(numn)
  {
    case 0:
      TJpgDec.drawJpg(numx,numy,W_1830_i0, sizeof(W_1830_i0));
      break;
    case 1:
      TJpgDec.drawJpg(numx,numy,W_1830_i1, sizeof(W_1830_i1));
      break;
    case 2:
      TJpgDec.drawJpg(numx,numy,W_1830_i2, sizeof(W_1830_i2));
      break;
    case 3:
      TJpgDec.drawJpg(numx,numy,W_1830_i3, sizeof(W_1830_i3));
      break;
    case 4:
      TJpgDec.drawJpg(numx,numy,W_1830_i4, sizeof(W_1830_i4));
      break;
    case 5:
      TJpgDec.drawJpg(numx,numy,W_1830_i5, sizeof(W_1830_i5));
      break;
    case 6:
      TJpgDec.drawJpg(numx,numy,W_1830_i6, sizeof(W_1830_i6));
      break;
    case 7:
      TJpgDec.drawJpg(numx,numy,W_1830_i7, sizeof(W_1830_i7));
      break;
    case 8:
      TJpgDec.drawJpg(numx,numy,W_1830_i8, sizeof(W_1830_i8));
      break;
    case 9:
      TJpgDec.drawJpg(numx,numy,W_1830_i9, sizeof(W_1830_i9));
      break;
    default:
      Serial.println("显示O_1830数字错误");
      break;
  }
}
