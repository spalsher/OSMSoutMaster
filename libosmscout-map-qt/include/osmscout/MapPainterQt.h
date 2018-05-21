#ifndef OSMSCOUT_MAP_MAPPAINTERQT_H
#define OSMSCOUT_MAP_MAPPAINTERQT_H

/*
  This source is part of the libosmscout-map library
  Copyright (C) 2010  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <mutex>

#include <QPainter>
#include <QMap>

#include <osmscout/MapQtImportExport.h>

#include <osmscout/MapPainter.h>

#include <QtGui/QTextLayout>

namespace osmscout {

  using QtGlyph = Glyph<QGlyphRun>;
  using QtLabel = Label<QGlyphRun, std::shared_ptr<QTextLayout>>;
  using QtLabelInstance = LabelInstance<QGlyphRun, std::shared_ptr<QTextLayout>>;

  class MapPainterBatchQt;

  /**
    Implementation of MapPainter for Qt
   */
  class OSMSCOUT_MAP_QT_API MapPainterQt : public MapPainter
  {
    friend class MapPainterBatchQt;

    using QtLabelLayouter = LabelLayouter<QGlyphRun, std::shared_ptr<QTextLayout>, MapPainterQt>;
    friend class LabelLayouter<QGlyphRun, std::shared_ptr<QTextLayout>, MapPainterQt>;

  private:
    struct FollowPathHandle
    {
      bool   closeWay;
      size_t transStart;
      size_t transEnd;
      size_t i;
      size_t nVertex;
      size_t direction;
    };
    struct FontDescriptor
    {
      QString       fontName;
      size_t        fontSize;
      QFont::Weight weight;
      bool          italic;

      bool operator<(const FontDescriptor& other) const
      {
        if (fontName!=other.fontName)
         return fontName<other.fontName;
        if (fontSize!=other.fontSize)
         return fontSize<other.fontSize;
        if (weight!=other.weight)
         return weight<other.weight;
        return italic<other.italic;
      }
    };

  private:
    QPainter                   *painter;

    QtLabelLayouter            labelLayouter;

    std::vector<QImage>        images;        //! vector of QImage for icons
    std::vector<QImage>        patternImages; //! vector of QImage for fill patterns
    std::vector<QBrush>        patterns;      //! vector of QBrush for fill patterns
    QMap<FontDescriptor,QFont> fonts;         //! Cached fonts
    std::vector<double>        sin;           //! Lookup table for sin calculation

    std::mutex                 mutex;         //! Mutex for locking concurrent calls

  private:
    QFont GetFont(const Projection& projection,
                  const MapParameter& parameter,
                  double fontSize);

    void SetFill(const Projection& projection,
                 const MapParameter& parameter,
                 const FillStyle& fillStyle);

    void SetBorder(const Projection& projection,
                   const MapParameter& parameter,
                   const BorderStyle& borderStyle);

    bool FollowPath(FollowPathHandle &hnd, double l, Vertex2D &origin);
    void FollowPathInit(FollowPathHandle &hnd, Vertex2D &origin, size_t transStart, size_t transEnd,
                        bool isClosed, bool keepOrientation);

    void SetupTransformation(QPainter* painter,
                             const QPointF center,
                             const qreal angle,
                             const qreal baseline) const;

    double GlyphWidth(const QGlyphRun &glyph);

    double GlyphHeight(const QGlyphRun &glyph);

    osmscout::Vertex2D GlyphTopLeft(const QGlyphRun &glyph);

    void DrawGlyph(QPainter *painter, const Glyph<QGlyphRun> &glyph) const;

    QtLabel Layout(std::string text, int fontSize, double proposedWidth);

  protected:
    bool HasIcon(const StyleConfig& styleConfig,
                 const MapParameter& parameter,
                 IconStyle& style) override;

    bool HasPattern(const MapParameter& parameter,
                    const FillStyle& style);

    double GetFontHeight(const Projection& projection,
                       const MapParameter& parameter,
                       double fontSize) override;

    // TextDimension GetTextDimension(const Projection& projection,
    //                                const MapParameter& parameter,
    //                                double objectWidth,
    //                                double fontSize,
    //                                const std::string& text) override;

    void DrawGround(const Projection& projection,
                    const MapParameter& parameter,
                    const FillStyle& style) override;

    // void DrawLabel(const Projection& projection,
    //                const MapParameter& parameter,
    //                const LabelData& label) override;

    /**
      Register regular label with given text at the given pixel coordinate
      in a style defined by the given LabelStyle.
     */
    virtual void RegisterRegularLabel(const Projection &projection,
                                      const MapParameter &parameter,
                                      const std::vector<LabelData> &labels,
                                      const Vertex2D &position,
                                      const double iconHeight = -1) override;

    /**
     * Register contour label
     */
    virtual void RegisterContourLabel(const Projection &projection,
                                      const MapParameter &parameter,
                                      const LabelData &label,
                                      const std::vector<Vertex2D> &way) override;

    virtual void DrawLabels(const Projection& projection,
                            const MapParameter& parameter,
                            const MapData& data) override;

    void DrawIcon(const IconStyle* style,
                  double x, double y) override;

    void DrawSymbol(const Projection& projection,
                    const MapParameter& parameter,
                    const Symbol& symbol,
                    double x, double y) override;

    void DrawPath(const Projection& projection,
                  const MapParameter& parameter,
                  const Color& color,
                  double width,
                  const std::vector<double>& dash,
                  LineStyle::CapStyle startCap,
                  LineStyle::CapStyle endCap,
                  size_t transStart, size_t transEnd) override;

    void DrawContourLabel(const Projection& projection,
                          const MapParameter& parameter,
                          const PathTextStyle& style,
                          const std::string& text,
                          size_t transStart, size_t transEnd,
                          ContourLabelHelper& helper) override;

    void DrawContourSymbol(const Projection& projection,
                           const MapParameter& parameter,
                           const Symbol& symbol,
                           double space,
                           size_t transStart, size_t transEnd) override;

    void DrawArea(const Projection& projection,
                  const MapParameter& parameter,
                  const AreaData& area) override;

  public:
    explicit MapPainterQt(const StyleConfigRef& styleConfig);
    ~MapPainterQt() override;

    void DrawGroundTiles(const Projection& projection,
                         const MapParameter& parameter,
                         const std::list<GroundTile>& groundTiles,
                         QPainter* painter);

    bool DrawMap(const Projection& projection,
                 const MapParameter& parameter,
                 const MapData& data,
                 QPainter* painter);
  };

  /**
   * \ingroup Renderer
   *
   * Qt specific MapPainterBatch. When given PainterQt instances
   * are used from multiple threads, they should be always
   * added in same order to avoid deadlocks.
   */
  class OSMSCOUT_MAP_QT_API MapPainterBatchQt:
      public MapPainterBatch<MapPainterQt*> {
  public:
    MapPainterBatchQt(size_t expectedCount);

    virtual ~MapPainterBatchQt();

    bool paint(const Projection& projection,
               const MapParameter& parameter,
               QPainter* painter);
  };
}

#endif
