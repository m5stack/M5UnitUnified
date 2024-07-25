
#ifndef SPRITE2_HPP
#define SPRITE2_HPP

#include <M5GFX.h>

class Sprite : public ::LGFX_Sprite
{
  public:
    explicit Sprite(::LovyanGFX* parent) : ::LGFX_Sprite(parent) {}
    Sprite() : LGFX_Sprite() {}

    /*!
      @brief Bit block transfer of color data corresponding to a pixel rectangle from the specified source sprite to the destination sprite.
      @param dst Destination sprite.
      @param dx X coodrdinate of the left-top corner of destination rectabgle.
      @param dy Y coodrdinate of the left-top corner of destination rectabgle.
      @param width Width of source and destination rectangle.
      @paran height Hidth of source and destination rectangle.
      @param sx X coodrdinate of the left-top corner of source rectabgle.
      @param sy Y coodrdinate of the left-top corner of source rectabgle.
      @param transp Transparent color
     */
    template<typename T>
    void pushPartial(Sprite* dst,
                     const int32_t dx, const int32_t dy,
                     const int32_t width, const int32_t height,
                     const int32_t sx, const int32_t sy,
                     const T& transp)
    {
        push_partial(dst, dx, dy, width, height, sx, sy, _write_conv.convert(transp) & _write_conv.colormask);
    }

    void pushPartial(Sprite* dst,
                     const int32_t dx, const int32_t dy,
                     const int32_t width, const int32_t height,
                     const int32_t sx, const int32_t sy)
    {
        push_partial(dst, dx, dy, width, height, sx, sy);
    }

    

    /*!
      @brief Bit block transfer of color data corresponding to a pixel rectangle from the specified source sprite to the destination sprite (helper)
      @param x X coodrdinate of the left-top corner of destination rectabgle.
      @param y Y coodrdinate of the left-top corner of destination rectabgle.
      @param width Width of source and destination rectangle.
      @param height Hidth of source and destination rectangle.
      @param pixelcopy data of source (SOURCE INFORMTION MUST ALREADY BE SET)
      @param use_dma using DMA?
    */
    void pushImagePartial(int32_t x, int32_t y, int32_t w, int32_t h, ::lgfx::pixelcopy_t *param, bool use_dma)
    {
        uint32_t x_mask = 7 >> (param->src_bits >> 1);
        param->src_bitwidth = (param->src_width + x_mask) & (~x_mask);

        int32_t dx=0, dw=w;
        if (0 < _clip_l - x) { dx = _clip_l - x; dw -= dx; x = _clip_l; }

        if (_adjust_width(x, dx, dw, _clip_l, _clip_r - _clip_l + 1)) return;
        int32_t dy=0, dh=h;
        if (0 < _clip_t - y) { dy = _clip_t - y; dh -= dy; y = _clip_t; }
        if (_adjust_width(y, dy, dh, _clip_t, _clip_b - _clip_t + 1)) return;

        startWrite();
        _panel->writeImage(x, y, dw, dh, param, use_dma);
        endWrite();
    }
    
  protected:
    /*!
      @brief Bit block transfer of color data corresponding to a pixel rectangle from the specified source sprite to the destination sprite.
      @param dst Destination sprite.
      @param x X coodrdinate of the left-top corner of destination rectabgle.
      @param y Y coodrdinate of the left-top corner of destination rectabgle.
      @param w Width of source and destination rectangle.
      @paran h Hidth of source and destination rectangle.
      @param sx X coodrdinate of the left-top corner of source rectabgle.
      @param sy Y coodrdinate of the left-top corner of source rectabgle.
      @param transp Transparent color
     */
    void push_partial(Sprite* dst,
                      const int32_t x, const int32_t y,
                      const int32_t w, const int32_t h,
                      int32_t sx,  int32_t sy,
                      const uint32_t transp = ::lgfx::pixelcopy_t::NON_TRANSP)
    {
        ::lgfx::pixelcopy_t p(_img, dst->getColorDepth(), getColorDepth(), dst->hasPalette(), _palette, transp);

        int32_t ssx=0, ssw=w;
        if (0 < _clip_l - sx) { ssx = _clip_l - sx; ssw -= ssx; sx = _clip_l; }
        if (_adjust_width(sx, ssx, ssw, _clip_l, _clip_r - _clip_l + 1)) return;
        int32_t ssy=0, ssh=h;
        if (0 < _clip_t - sy) { ssy = _clip_t - sy; ssh -= ssy; sy = _clip_t; }
        if (_adjust_width(sy, ssy, ssh, _clip_t, _clip_b - _clip_t + 1)) return;
        
        p.src_width = width();
        p.src_height = height();
        p.src_x32 = p.src_x32_add * sx;
        p.src_y = sy;
        dst->pushImagePartial(x, y, ssw, ssh, &p, _panel_sprite.getSpriteBuffer()->use_dma());
    }
};

#endif
