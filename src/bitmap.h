#define TRIG_MAX (1<<24)
#define TRIG_NORM(v) (v>>24)

const int32_t _sin_[91] = {
0, 292802, 585516, 878051, 1170319, 1462230, 1753696, 2044628, 2334937, 2624534, 2913332, 3201243, 3488179, 3774052, 4058775, 4342263, 4624427, 4905183, 5184444, 5462127, 5738145, 6012416, 6284855, 6555380, 6823908, 7090357, 7354647, 7616696, 7876425, 8133755, 8388607, 8640905, 8890569, 9137526, 9381700, 9623015, 9861400, 10096780, 10329085, 10558244, 10784186, 11006844, 11226148, 11442033, 11654433, 11863283, 12068519, 12270079, 12467901, 12661925, 12852093, 13038345, 13220626, 13398880, 13573052, 13743090, 13908942, 14070557, 14227886, 14380880, 14529495, 14673683, 14813402, 14948608, 15079261, 15205321, 15326749, 15443508, 15555563, 15662880, 15765426, 15863169, 15956080, 16044131, 16127295, 16205546, 16278860, 16347217, 16410593, 16468971, 16522332, 16570660, 16613941, 16652161, 16685308, 16713373, 16736347, 16754223, 16766995, 16774660, 16777216
};


static inline int32_t _SIN(int d) {
    d = d%360;
    if (d < 90) {
        return _sin_[d];
    } else if (d < 180) {
        return _sin_[180-d];
    } else if (d < 270) {
        return -_sin_[d-180];
    } else {
        return -_sin_[360-d];
    }
}

static inline int32_t _COS(int d) {
    d = d%360;
    if (d < 90) {
        return _sin_[90-d];
    } else if (d < 180) {
        return -_sin_[d-90];
    } else if (d < 270) {
        return -_sin_[270-d];
    } else {
        return _sin_[d-270];
    }
}

static inline GColor bmpGetPixel(const GBitmap *bmp, int x, int y) {
    if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return -1;
    int byteoffset = y*bmp->row_size_bytes + x/8;
    return ((((uint8_t *)bmp->addr)[byteoffset] & (1<<(x%8))) != 0);
}

static inline void bmpPutPixel(GBitmap *bmp, int x, int y, GColor c) {
    if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return;
    int byteoffset = y*bmp->row_size_bytes + x/8;
    ((uint8_t *)bmp->addr)[byteoffset] &= ~(1<<(x%8));
    if (c == GColorWhite) ((uint8_t *)bmp->addr)[byteoffset] |= (1<<(x%8));
}

static void bmpNegative(GBitmap *bmp) {
    for (int i=0; i<bmp->row_size_bytes*bmp->bounds.size.h; i++) ((uint8_t *)bmp->addr)[i] = ~((uint8_t *)bmp->addr)[i];
}

static void bmpCopy(const GBitmap *src, GBitmap *dst) {
    for (int y=0; y<dst->bounds.size.h; y++) {
        for (int x=0; x<dst->bounds.size.w; x++) {
            bmpPutPixel(dst, x, y, bmpGetPixel(src, x, y));
        }
    }
}

static void bmpSub(const GBitmap *src, GBitmap *dst, GRect from, GPoint to) {
    for (int y=0; y<from.size.h; y++) {
        for (int x=0; x<from.size.w; x++) {
            bmpPutPixel(dst, to.x+x, to.y+y, bmpGetPixel(src, from.origin.x+x, from.origin.y+y));
        }
    }
}

static inline void bmpFillRect(GBitmap *bmp, GRect rect, GColor c) {
    int i, j;
    int xe = rect.origin.x + rect.size.w;
    int ye = rect.origin.y + rect.size.h;
 
    for (j=rect.origin.y ; j<ye; j++) {
        for (i=rect.origin.x ; i<xe; i++) {
            bmpPutPixel(bmp, i, j, c);
        }
    }    
}


static void bmpDrawLine(GBitmap *bmp, GPoint p1, GPoint p2, GColor c) {
	int dx, dy, e;
    
	if ((dx = p2.x-p1.x) != 0) {
		if (dx > 0) {
			if ((dy = p2.y-p1.y) != 0) {
				if (dy > 0) {
					// vecteur oblique dans le 1er quadran
					if (dx >= dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 1er octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x++;
							if (p1.x == p2.x) break;
							e -= dy;
							if (e < 0) {
								p1.y++;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 2nd octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y++;
							if (p1.y == p2.y) break;
							e -= dx;
							if (e < 0) {
								p1.x++;
								e += dy;
							}
						}
					}
				} else { // dy < 0 (et dx > 0)
					// vecteur oblique dans le 4e cadran
					if (dx >= -dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 8e octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x++;
							if (p1.x == p2.x) break;
							e += dy;
							if (e < 0) {
								p1.y--;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 7e octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y--;
							if (p1.y == p2.y) break;
							e += dx;
							if (e > 0) {
								p1.x++;
								e += dy;
							}
						}
					}
				}
			} else {
				// dy = 0 (et dx > 0)
                // vecteur horizontal vers la droite
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.x++;
					if (p1.x == p2.x) break;
				}
			}
		} else {
			// dx < 0
			if ((dy = p2.y-p1.y) != 0) {
				if (dy > 0) {
					// vecteur oblique dans le 2nd quadran
					if (-dx >= dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 4e octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x--;
							if (p1.x == p2.x) break;
							e += dy;
							if (e >= 0) {
								p1.y++;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 3e octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y++;
							if (p1.y == p2.y) break;
							e += dx;
							if (e <= 0) {
								p1.x--;
								e += dy;
							}
						}
					}
				} else {
					// dy < 0 (et dx < 0)
					// vecteur oblique dans le 3e cadran
                    if (dx <= dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 5e octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x--;
							if (p1.x == p2.x) break;
							e -= dy;
							if (e >= 0) {
								p1.y--;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 6e octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y--;
							if (p1.y == p2.y) break;
							e -= dx;
							if (e >= 0) {
								p1.x--;
								e += dy;
							}
						}
					}
				}
			} else {
				// dy = 0 (et dx < 0)
				// vecteur horizontal vers la gauche
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.x--;
					if (p1.x == p2.x) break;
				}
			}
		}
	} else {
		// dx = 0
		if ((dy = p2.y-p1.y) != 0) {
			if (dy > 0) {
				// vecteur vertical croissant
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.y++;
					if (p1.y == p2.y) break;
				}
			} else {
				// dy < 0 (et dx = 0)
				// vecteur vertical décroissant
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.y--;
					if (p1.y == p2.y) break;
				}
            }
		}
	}
}

static void bmpDrawCircle(GBitmap *bmp, GPoint center, int r, GColor c) {
	int x = 0, y = r, d = r-1;
    
	while (y >= x) {
		bmpPutPixel(bmp, center.x+x, center.y+y, c);
		bmpPutPixel(bmp, center.x+y, center.y+x, c);
		bmpPutPixel(bmp, center.x-x, center.y+y, c);
		bmpPutPixel(bmp, center.x-y, center.y+x, c);
		bmpPutPixel(bmp, center.x+x, center.y-y, c);
		bmpPutPixel(bmp, center.x+y, center.y-x, c);
		bmpPutPixel(bmp, center.x-x, center.y-y, c);
		bmpPutPixel(bmp, center.x-y, center.y-x, c);
        
		if (d >= 2*x-2) {
			d = d-2*x;
			x++;
		} else if (d <= 2*r - 2*y) {
			d = d+2*y-1;
			y--;
		} else {
			d = d + 2*y - 2*x - 2;
			y--;
			x++;
		}
	}
}

static void bmpFillCircle(GBitmap *bmp, GPoint center, int r, GColor c) {
	int x = 0, y = r, d = r-1, v;
    
	while (y >= x) {
        for (v=center.x-x; v<=center.x+x; v++) bmpPutPixel(bmp, v, center.y+y, c);
        for (v=center.x-y; v<=center.x+y; v++) bmpPutPixel(bmp, v, center.y+x, c);
        for (v=center.x-x; v<=center.x+x; v++) bmpPutPixel(bmp, v, center.y-y, c);
        for (v=center.x-y; v<=center.x+y; v++) bmpPutPixel(bmp, v, center.y-x, c);
        
		if (d >= 2*x-2) {
			d = d-2*x;
			x++;
		} else if (d <= 2*r - 2*y) {
			d = d+2*y-1;
			y--;
		} else {
			d = d + 2*y - 2*x - 2;
			y--;
			x++;
		}
	}
}

static void bmpRotate(const GBitmap *src, GBitmap *dst, int a, GPoint srcCenter, GPoint dstOffset) {
	int w = dst->bounds.size.w;
	int h = dst->bounds.size.h;
	int x, y, xx, yy, c;
	int32_t cosphi, sinphi, xo, yo, rx, ry;
    
	a = a%360;
    a = 360-a;
    
    cosphi = _COS(a);
    sinphi = _SIN(a);
    
    for (x=0; x<w; x++) {
        for (y=0; y<h; y++) {
            xo = x - dstOffset.x - srcCenter.x;
            yo = y - dstOffset.x - srcCenter.y;
            rx = TRIG_NORM(xo*cosphi) - TRIG_NORM(yo*sinphi);
            ry = TRIG_NORM(xo*sinphi) + TRIG_NORM(yo*cosphi);
            xx = rx + srcCenter.x;
            yy = ry + srcCenter.y;
            c = bmpGetPixel(src, xx, yy);
            if (c >= 0) bmpPutPixel(dst, x, y, c);
        }
    }
}

