/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qpicture.h"
#include <private/qpicture_p.h>

#ifndef QT_NO_PICTURE

#include <private/qfactoryloader_p.h>
#include <private/qpaintengine_pic_p.h>
#include <private/qfont_p.h>
#include <qguiapplication.h>

#include "qdatastream.h"
#include "qfile.h"
#include "qimage.h"
#include "qmutex.h"
#include "qpainter.h"
#include "qpainterpath.h"
#include "qpixmap.h"
#include "qregexp.h"
#include "qregion.h"
#include "qdebug.h"
#include <QtCore/private/qlocking_p.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

void qt_format_text(const QFont &fnt, const QRectF &_r,
                    int tf, const QTextOption *opt, const QString& str, QRectF *brect,
                    int tabstops, int *, int tabarraylen,
                    QPainter *painter);

/*!
    \class QPicture
    \brief The QPicture class is a paint device that records and
    replays QPainter commands.

    \inmodule QtGui
    \ingroup shared


    A picture serializes painter commands to an IO device in a
    platform-independent format. They are sometimes referred to as meta-files.

    Qt pictures use a proprietary binary format. Unlike native picture
    (meta-file) formats on many window systems, Qt pictures have no
    limitations regarding their contents. Everything that can be
    painted on a widget or pixmap (e.g., fonts, pixmaps, regions,
    transformed graphics, etc.)  can also be stored in a picture.

    QPicture is resolution independent, i.e. a QPicture can be
    displayed on different devices (for example svg, pdf, ps, printer
    and screen) looking the same. This is, for instance, needed for
    WYSIWYG print preview. QPicture runs in the default system dpi,
    and scales the painter to match differences in resolution
    depending on the window system.

    Example of how to record a picture:
    \snippet picture/picture.cpp 0

    Note that the list of painter commands is reset on each call to
    the QPainter::begin() function.

    Example of how to replay a picture:
    \snippet picture/picture.cpp 1

    Pictures can also be drawn using play(). Some basic data about a
    picture is available, for example, size(), isNull() and
    boundingRect().

    \sa QMovie
*/

/*!
    \fn QPicture &QPicture::operator=(QPicture &&other)

    Move-assigns \a other to this QPicture instance.

    \since 5.2
*/

const char  *qt_mfhdr_tag = "QPIC"; // header tag
static const quint16 mfhdr_maj = QDataStream::Qt_DefaultCompiledVersion; // major version #
static const quint16 mfhdr_min = 0; // minor version #

/*!
    Constructs an empty picture.

    The \a formatVersion parameter may be used to \e create a QPicture
    that can be read by applications that are compiled with earlier
    versions of Qt.

    Note that the default formatVersion is -1 which signifies the
    current release, i.e. for Qt 4.0 a formatVersion of 7 is the same
    as the default formatVersion of -1.

    Reading pictures generated by earlier versions of Qt is not
    supported in Qt 4.0.
*/

QPicture::QPicture(int formatVersion)
    : QPaintDevice(),
      d_ptr(new QPicturePrivate)
{
    Q_D(QPicture);

    if (formatVersion == 0)
        qWarning("QPicture: invalid format version 0");

    // still accept the 0 default from before Qt 3.0.
    if (formatVersion > 0 && formatVersion != (int)mfhdr_maj) {
        d->formatMajor = formatVersion;
        d->formatMinor = 0;
        d->formatOk = false;
    } else {
        d->resetFormat();
    }
}

/*!
    Constructs a copy of \a pic.

    This constructor is fast thanks to \l{implicit sharing}.
*/

QPicture::QPicture(const QPicture &pic)
    : QPaintDevice(), d_ptr(pic.d_ptr)
{
}

/*! \internal */
QPicture::QPicture(QPicturePrivate &dptr)
    : QPaintDevice(),
      d_ptr(&dptr)
{
}

/*!
    Destroys the picture.
*/
QPicture::~QPicture()
{
}

/*!
  \internal
*/
int QPicture::devType() const
{
    return QInternal::Picture;
}

/*!
    \fn bool QPicture::isNull() const

    Returns \c true if the picture contains no data; otherwise returns
    false.
*/

/*!
    \fn uint QPicture::size() const

    Returns the size of the picture data.

    \sa data()
*/

/*!
    \fn const char* QPicture::data() const

    Returns a pointer to the picture data. The pointer is only valid
    until the next non-const function is called on this picture. The
    returned pointer is 0 if the picture contains no data.

    \sa size(), isNull()
*/


bool QPicture::isNull() const
{
    return d_func()->pictb.buffer().isNull();
}

uint QPicture::size() const
{
    return d_func()->pictb.buffer().size();
}

const char* QPicture::data() const
{
    return d_func()->pictb.buffer();
}

void QPicture::detach()
{
    d_ptr.detach();
}

bool QPicture::isDetached() const
{
    return d_func()->ref.loadRelaxed() == 1;
}

/*!
    Sets the picture data directly from \a data and \a size. This
    function copies the input data.

    \sa data(), size()
*/

void QPicture::setData(const char* data, uint size)
{
    detach();
    d_func()->pictb.setData(data, size);
    d_func()->resetFormat();                                // we'll have to check
}


/*!
    Loads a picture from the file specified by \a fileName and returns
    true if successful; otherwise invalidates the picture and returns \c false.

    Please note that the \a format parameter has been deprecated and
    will have no effect.

    \sa save()
*/

bool QPicture::load(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        operator=(QPicture());
        return false;
    }
    return load(&f);
}

/*!
    \overload

    \a dev is the device to use for loading.
*/

bool QPicture::load(QIODevice *dev)
{
    detach();
    QByteArray a = dev->readAll();

    d_func()->pictb.setData(a);                        // set byte array in buffer
    return d_func()->checkFormat();
}

/*!
    Saves a picture to the file specified by \a fileName and returns
    true if successful; otherwise returns \c false.

    Please note that the \a format parameter has been deprecated and
    will have no effect.

    \sa load()
*/

bool QPicture::save(const QString &fileName)
{
    if (paintingActive()) {
        qWarning("QPicture::save: still being painted on. "
                  "Call QPainter::end() first");
        return false;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    return save(&f);
}

/*!
    \overload

    \a dev is the device to use for saving.
*/

bool QPicture::save(QIODevice *dev)
{
    if (paintingActive()) {
        qWarning("QPicture::save: still being painted on. "
                  "Call QPainter::end() first");
        return false;
    }

    dev->write(d_func()->pictb.buffer(), d_func()->pictb.buffer().size());
    return true;
}

/*!
    Returns the picture's bounding rectangle or an invalid rectangle
    if the picture contains no data.
*/

QRect QPicture::boundingRect() const
{
    Q_D(const QPicture);
    // Use override rect where possible.
    if (!d->override_rect.isEmpty())
        return d->override_rect;

    if (!d->formatOk)
        d_ptr->checkFormat();

    return d->brect;
}

/*!
    Sets the picture's bounding rectangle to \a r. The automatically
    calculated value is overridden.
*/

void QPicture::setBoundingRect(const QRect &r)
{
    d_func()->override_rect = r;
}

/*!
    Replays the picture using \a painter, and returns \c true if
    successful; otherwise returns \c false.

    This function does exactly the same as QPainter::drawPicture()
    with (x, y) = (0, 0).
*/

bool QPicture::play(QPainter *painter)
{
    Q_D(QPicture);

    if (d->pictb.size() == 0)                        // nothing recorded
        return true;

    if (!d->formatOk && !d->checkFormat())
        return false;

    d->pictb.open(QIODevice::ReadOnly);                // open buffer device
    QDataStream s;
    s.setDevice(&d->pictb);                        // attach data stream to buffer
    s.device()->seek(10);                        // go directly to the data
    s.setVersion(d->formatMajor == 4 ? 3 : d->formatMajor);

    quint8  c, clen;
    quint32 nrecords;
    s >> c >> clen;
    Q_ASSERT(c == QPicturePrivate::PdcBegin);
    // bounding rect was introduced in ver 4. Read in checkFormat().
    if (d->formatMajor >= 4) {
        qint32 dummy;
        s >> dummy >> dummy >> dummy >> dummy;
    }
    s >> nrecords;
    if (!exec(painter, s, nrecords)) {
        qWarning("QPicture::play: Format error");
        d->pictb.close();
        return false;
    }
    d->pictb.close();
    return true;                                // no end-command
}


//
// QFakeDevice is used to create fonts with a custom DPI
//
class QFakeDevice : public QPaintDevice
{
public:
    QFakeDevice() { dpi_x = qt_defaultDpiX(); dpi_y = qt_defaultDpiY(); }
    void setDpiX(int dpi) { dpi_x = dpi; }
    void setDpiY(int dpi) { dpi_y = dpi; }
    QPaintEngine *paintEngine() const override { return 0; }
    int metric(PaintDeviceMetric m) const override
    {
        switch(m) {
            case PdmPhysicalDpiX:
            case PdmDpiX:
                return dpi_x;
            case PdmPhysicalDpiY:
            case PdmDpiY:
                return dpi_y;
            default:
                return QPaintDevice::metric(m);
        }
    }

private:
    int dpi_x;
    int dpi_y;
};

/*!
  \internal
  Iterates over the internal picture data and draws the picture using
  \a painter.
*/

bool QPicture::exec(QPainter *painter, QDataStream &s, int nrecords)
{
    Q_D(QPicture);
#if defined(QT_DEBUG)
    int                strm_pos;
#endif
    quint8     c;                      // command id
    quint8     tiny_len;               // 8-bit length descriptor
    qint32     len;                    // 32-bit length descriptor
    qint16     i_16, i1_16, i2_16;     // parameters...
    qint8      i_8;
    quint32    ul;
    double     dbl;
    bool       bl;
    QByteArray  str1;
    QString     str;
    QPointF     p, p1, p2;
    QPoint      ip, ip1, ip2;
    QRect       ir;
    QRectF      r;
    QPolygonF   a;
    QPolygon    ia;
    QColor      color;
    QFont       font;
    QPen        pen;
    QBrush      brush;
    QRegion     rgn;
    QMatrix     wmatrix;
    QTransform  matrix;

    QTransform worldMatrix = painter->transform();
    worldMatrix.scale(qreal(painter->device()->logicalDpiX()) / qreal(qt_defaultDpiX()),
                      qreal(painter->device()->logicalDpiY()) / qreal(qt_defaultDpiY()));
    painter->setTransform(worldMatrix);

    while (nrecords-- && !s.atEnd()) {
        s >> c;                 // read cmd
        s >> tiny_len;          // read param length
        if (tiny_len == 255)    // longer than 254 bytes
            s >> len;
        else
            len = tiny_len;
#if defined(QT_DEBUG)
        strm_pos = s.device()->pos();
#endif
        switch (c) {            // exec cmd
        case QPicturePrivate::PdcNOP:
            break;
        case QPicturePrivate::PdcDrawPoint:
            if (d->formatMajor <= 5) {
                s >> ip;
                painter->drawPoint(ip);
            } else {
                s >> p;
                painter->drawPoint(p);
            }
            break;
        case QPicturePrivate::PdcDrawPoints:
// ## implement me in the picture paint engine
//                 s >> a >> i1_32 >> i2_32;
//                 painter->drawPoints(a.mid(i1_32, i2_32));
            break;
        case QPicturePrivate::PdcDrawPath: {
            QPainterPath path;
            s >> path;
            painter->drawPath(path);
            break;
        }
        case QPicturePrivate::PdcDrawLine:
            if (d->formatMajor <= 5) {
                s >> ip1 >> ip2;
                painter->drawLine(ip1, ip2);
            } else {
                s >> p1 >> p2;
                painter->drawLine(p1, p2);
            }
            break;
        case QPicturePrivate::PdcDrawRect:
            if (d->formatMajor <= 5) {
                s >> ir;
                painter->drawRect(ir);
            } else {
                s >> r;
                painter->drawRect(r);
            }
            break;
        case QPicturePrivate::PdcDrawRoundRect:
            if (d->formatMajor <= 5) {
                s >> ir >> i1_16 >> i2_16;
                painter->drawRoundedRect(ir, i1_16, i2_16, Qt::RelativeSize);
            } else {
                s >> r >> i1_16 >> i2_16;
                painter->drawRoundedRect(r, i1_16, i2_16, Qt::RelativeSize);
            }
            break;
        case QPicturePrivate::PdcDrawEllipse:
            if (d->formatMajor <= 5) {
                s >> ir;
                painter->drawEllipse(ir);
            } else {
                s >> r;
                painter->drawEllipse(r);
            }
            break;
        case QPicturePrivate::PdcDrawArc:
            if (d->formatMajor <= 5) {
                s >> ir;
                r = ir;
            } else {
                s >> r;
            }
            s >> i1_16 >> i2_16;
            painter->drawArc(r, i1_16, i2_16);
            break;
        case QPicturePrivate::PdcDrawPie:
            if (d->formatMajor <= 5) {
                s >> ir;
                r = ir;
            } else {
                s >> r;
            }
            s >> i1_16 >> i2_16;
            painter->drawPie(r, i1_16, i2_16);
            break;
        case QPicturePrivate::PdcDrawChord:
            if (d->formatMajor <= 5) {
                s >> ir;
                r = ir;
            } else {
                s >> r;
            }
            s >> i1_16 >> i2_16;
            painter->drawChord(r, i1_16, i2_16);
            break;
        case QPicturePrivate::PdcDrawLineSegments:
            s >> ia;
            painter->drawLines(ia);
            ia.clear();
            break;
        case QPicturePrivate::PdcDrawPolyline:
            if (d->formatMajor <= 5) {
                s >> ia;
                painter->drawPolyline(ia);
                ia.clear();
            } else {
                s >> a;
                painter->drawPolyline(a);
                a.clear();
            }
            break;
        case QPicturePrivate::PdcDrawPolygon:
            if (d->formatMajor <= 5) {
                s >> ia >> i_8;
                painter->drawPolygon(ia, i_8 ? Qt::WindingFill : Qt::OddEvenFill);
                ia.clear();
            } else {
                s >> a >> i_8;
                painter->drawPolygon(a, i_8 ? Qt::WindingFill : Qt::OddEvenFill);
                a.clear();
            }
            break;
        case QPicturePrivate::PdcDrawCubicBezier: {
            s >> ia;
            QPainterPath path;
            Q_ASSERT(ia.size() == 4);
            path.moveTo(ia.value(0));
            path.cubicTo(ia.value(1), ia.value(2), ia.value(3));
            painter->strokePath(path, painter->pen());
            ia.clear();
        }
            break;
        case QPicturePrivate::PdcDrawText:
            s >> ip >> str1;
            painter->drawText(ip, QString::fromLatin1(str1));
            break;
        case QPicturePrivate::PdcDrawTextFormatted:
            s >> ir >> i_16 >> str1;
            painter->drawText(ir, i_16, QString::fromLatin1(str1));
            break;
        case QPicturePrivate::PdcDrawText2:
            if (d->formatMajor <= 5) {
                s >> ip >> str;
                painter->drawText(ip, str);
            } else {
                s >> p >> str;
                painter->drawText(p, str);
            }
            break;
        case QPicturePrivate::PdcDrawText2Formatted:
            s >> ir;
            s >> i_16;
            s >> str;
            painter->drawText(ir, i_16, str);
            break;
        case QPicturePrivate::PdcDrawTextItem: {
            s >> p >> str >> font >> ul;

            // the text layout direction is not used here because it's already
            // aligned when QPicturePaintEngine::drawTextItem() serializes the
            // drawText() call, therefore ul is unsed in this context

            if (d->formatMajor >= 9) {
                s >> dbl;
                QFont fnt(font);
                if (dbl != 1.0) {
                    QFakeDevice fake;
                    fake.setDpiX(qRound(dbl*qt_defaultDpiX()));
                    fake.setDpiY(qRound(dbl*qt_defaultDpiY()));
                    fnt = QFont(font, &fake);
                }

                qreal justificationWidth;
                s >> justificationWidth;

                int flags = Qt::TextSingleLine | Qt::TextDontClip | Qt::TextForceLeftToRight;

                QSizeF size(1, 1);
                if (justificationWidth > 0) {
                    size.setWidth(justificationWidth);
                    flags |= Qt::TextJustificationForced;
                    flags |= Qt::AlignJustify;
                }

                QFontMetrics fm(fnt);
                QPointF pt(p.x(), p.y() - fm.ascent());
                qt_format_text(fnt, QRectF(pt, size), flags, /*opt*/0,
                               str, /*brect=*/0, /*tabstops=*/0, /*...*/0, /*tabarraylen=*/0, painter);
            } else {
                qt_format_text(font, QRectF(p, QSizeF(1, 1)), Qt::TextSingleLine | Qt::TextDontClip, /*opt*/0,
                               str, /*brect=*/0, /*tabstops=*/0, /*...*/0, /*tabarraylen=*/0, painter);
            }

            break;
        }
        case QPicturePrivate::PdcDrawPixmap: {
            QPixmap pixmap;
            if (d->formatMajor < 4) {
                s >> ip >> pixmap;
                painter->drawPixmap(ip, pixmap);
            } else if (d->formatMajor <= 5) {
                s >> ir >> pixmap;
                painter->drawPixmap(ir, pixmap);
            } else {
                QRectF sr;
                if (d->in_memory_only) {
                    int index;
                    s >> r >> index >> sr;
                    Q_ASSERT(index < d->pixmap_list.size());
                    pixmap = d->pixmap_list.value(index);
                } else {
                    s >> r >> pixmap >> sr;
                }
                painter->drawPixmap(r, pixmap, sr);
            }
        }
            break;
        case QPicturePrivate::PdcDrawTiledPixmap: {
            QPixmap pixmap;
            if (d->in_memory_only) {
                int index;
                s >> r >> index >> p;
                Q_ASSERT(index < d->pixmap_list.size());
                pixmap = d->pixmap_list.value(index);
            } else {
                s >> r >> pixmap >> p;
            }
            painter->drawTiledPixmap(r, pixmap, p);
        }
            break;
        case QPicturePrivate::PdcDrawImage: {
            QImage image;
            if (d->formatMajor < 4) {
                s >> p >> image;
                painter->drawImage(p, image);
            } else if (d->formatMajor <= 5){
                s >> ir >> image;
                painter->drawImage(ir, image, QRect(0, 0, ir.width(), ir.height()));
            } else {
                QRectF sr;
                if (d->in_memory_only) {
                    int index;
                    s >> r >> index >> sr >> ul;
                    Q_ASSERT(index < d->image_list.size());
                    image = d->image_list.value(index);
                } else {
                    s >> r >> image >> sr >> ul;
                }
                painter->drawImage(r, image, sr, Qt::ImageConversionFlags(ul));
            }
        }
            break;
        case QPicturePrivate::PdcBegin:
            s >> ul;                        // number of records
            if (!exec(painter, s, ul))
                return false;
            break;
        case QPicturePrivate::PdcEnd:
            if (nrecords == 0)
                return true;
            break;
        case QPicturePrivate::PdcSave:
            painter->save();
            break;
        case QPicturePrivate::PdcRestore:
            painter->restore();
            break;
        case QPicturePrivate::PdcSetBkColor:
            s >> color;
            painter->setBackground(color);
            break;
        case QPicturePrivate::PdcSetBkMode:
            s >> i_8;
            painter->setBackgroundMode((Qt::BGMode)i_8);
            break;
        case QPicturePrivate::PdcSetROP: // NOP
            s >> i_8;
            break;
        case QPicturePrivate::PdcSetBrushOrigin:
            if (d->formatMajor <= 5) {
                s >> ip;
                painter->setBrushOrigin(ip);
            } else {
                s >> p;
                painter->setBrushOrigin(p);
            }
            break;
        case QPicturePrivate::PdcSetFont:
            s >> font;
            painter->setFont(font);
            break;
        case QPicturePrivate::PdcSetPen:
            if (d->in_memory_only) {
                int index;
                s >> index;
                Q_ASSERT(index < d->pen_list.size());
                pen = d->pen_list.value(index);
            } else {
                s >> pen;
            }
            painter->setPen(pen);
            break;
        case QPicturePrivate::PdcSetBrush:
            if (d->in_memory_only) {
                int index;
                s >> index;
                Q_ASSERT(index < d->brush_list.size());
                brush = d->brush_list.value(index);
            } else {
                s >> brush;
            }
            painter->setBrush(brush);
            break;
        case QPicturePrivate::PdcSetVXform:
            s >> i_8;
            painter->setViewTransformEnabled(i_8);
            break;
        case QPicturePrivate::PdcSetWindow:
            if (d->formatMajor <= 5) {
                s >> ir;
                painter->setWindow(ir);
            } else {
                s >> r;
                painter->setWindow(r.toRect());
            }
            break;
        case QPicturePrivate::PdcSetViewport:
            if (d->formatMajor <= 5) {
                s >> ir;
                painter->setViewport(ir);
            } else {
                s >> r;
                painter->setViewport(r.toRect());
            }
            break;
        case QPicturePrivate::PdcSetWXform:
            s >> i_8;
            painter->setWorldMatrixEnabled(i_8);
            break;
        case QPicturePrivate::PdcSetWMatrix:
            if (d->formatMajor >= 8) {
                s >> matrix >> i_8;
            } else {
                s >> wmatrix >> i_8;
                matrix = QTransform(wmatrix);
            }
            // i_8 is always false due to updateXForm() in qpaintengine_pic.cpp
            painter->setTransform(matrix * worldMatrix, i_8);
            break;
        case QPicturePrivate::PdcSetClip:
            s >> i_8;
            painter->setClipping(i_8);
            break;
        case QPicturePrivate::PdcSetClipRegion:
            s >> rgn >> i_8;
            if (d->formatMajor >= 9) {
                painter->setClipRegion(rgn, Qt::ClipOperation(i_8));
            } else {
                painter->setClipRegion(rgn);
            }
            break;
        case QPicturePrivate::PdcSetClipPath:
            {
                QPainterPath path;
                s >> path >> i_8;
                painter->setClipPath(path, Qt::ClipOperation(i_8));
                break;
            }
        case QPicturePrivate::PdcSetRenderHint:
            s >> ul;
            painter->setRenderHint(QPainter::Antialiasing,
                                   bool(ul & QPainter::Antialiasing));
            painter->setRenderHint(QPainter::SmoothPixmapTransform,
                                   bool(ul & QPainter::SmoothPixmapTransform));
            break;
        case QPicturePrivate::PdcSetCompositionMode:
            s >> ul;
            painter->setCompositionMode((QPainter::CompositionMode)ul);
            break;
        case QPicturePrivate::PdcSetClipEnabled:
            s >> bl;
            painter->setClipping(bl);
            break;
        case QPicturePrivate::PdcSetOpacity:
            s >> dbl;
            painter->setOpacity(qreal(dbl));
            break;
        default:
            qWarning("QPicture::play: Invalid command %d", c);
            if (len > 0)                    // skip unknown command
                s.device()->seek(s.device()->pos()+len);
        }
#if defined(QT_DEBUG)
        //qDebug("device->at(): %i, strm_pos: %i len: %i", (int)s.device()->pos(), strm_pos, len);
        Q_ASSERT(qint32(s.device()->pos() - strm_pos) == len);
#endif
    }
    return false;
}

/*!
    \internal

    Internal implementation of the virtual QPaintDevice::metric()
    function.

    A picture has the following hard-coded values: numcolors=16777216
    and depth=24.

    \a m is the metric to get.
*/

int QPicture::metric(PaintDeviceMetric m) const
{
    int val;
    QRect brect = boundingRect();
    switch (m) {
        case PdmWidth:
            val = brect.width();
            break;
        case PdmHeight:
            val = brect.height();
            break;
        case PdmWidthMM:
            val = int(25.4/qt_defaultDpiX()*brect.width());
            break;
        case PdmHeightMM:
            val = int(25.4/qt_defaultDpiY()*brect.height());
            break;
        case PdmDpiX:
        case PdmPhysicalDpiX:
            val = qt_defaultDpiX();
            break;
        case PdmDpiY:
        case PdmPhysicalDpiY:
            val = qt_defaultDpiY();
            break;
        case PdmNumColors:
            val = 16777216;
            break;
        case PdmDepth:
            val = 24;
            break;
        case PdmDevicePixelRatio:
            val = 1;
            break;
        case PdmDevicePixelRatioScaled:
            val = 1 * QPaintDevice::devicePixelRatioFScale();
            break;
        default:
            val = 0;
            qWarning("QPicture::metric: Invalid metric command");
    }
    return val;
}

/*!
    \fn void QPicture::detach()
    \internal
    Detaches from shared picture data and makes sure that this picture
    is the only one referring to the data.

    If multiple pictures share common data, this picture makes a copy
    of the data and detaches itself from the sharing mechanism.
    Nothing is done if there is just a single reference.
*/

/*! \fn bool QPicture::isDetached() const
\internal
*/

/*!
    Assigns picture \a p to this picture and returns a reference to
    this picture.
*/
QPicture& QPicture::operator=(const QPicture &p)
{
    d_ptr = p.d_ptr;
    return *this;
}

/*!
    \fn void QPicture::swap(QPicture &other)
    \since 4.8

    Swaps picture \a other with this picture. This operation is very
    fast and never fails.
*/

/*!
  \internal

  Constructs a QPicturePrivate
*/
QPicturePrivate::QPicturePrivate()
    : in_memory_only(false)
{
}

/*!
  \internal

  Copy-Constructs a QPicturePrivate. Needed when detaching.
*/
QPicturePrivate::QPicturePrivate(const QPicturePrivate &other)
    : trecs(other.trecs),
      formatOk(other.formatOk),
      formatMinor(other.formatMinor),
      brect(other.brect),
      override_rect(other.override_rect),
      in_memory_only(false)
{
    pictb.setData(other.pictb.data(), other.pictb.size());
    if (other.pictb.isOpen()) {
        pictb.open(other.pictb.openMode());
        pictb.seek(other.pictb.pos());
    }
}

/*!
  \internal

  Sets formatOk to false and resets the format version numbers to default
*/

void QPicturePrivate::resetFormat()
{
    formatOk = false;
    formatMajor = mfhdr_maj;
    formatMinor = mfhdr_min;
}


/*!
  \internal

  Checks data integrity and format version number. Set formatOk to
  true on success, to false otherwise. Returns the resulting formatOk
  value.
*/
bool QPicturePrivate::checkFormat()
{
    resetFormat();

    // can't check anything in an empty buffer
    if (pictb.size() == 0 || pictb.isOpen())
        return false;

    pictb.open(QIODevice::ReadOnly);                        // open buffer device
    QDataStream s;
    s.setDevice(&pictb);                        // attach data stream to buffer

    char mf_id[4];                                // picture header tag
    s.readRawData(mf_id, 4);                        // read actual tag
    int bufSize = pictb.buffer().size();
    if (memcmp(mf_id, qt_mfhdr_tag, 4) != 0 || bufSize < 12) {   // wrong header id or size
        qWarning("QPicturePaintEngine::checkFormat: Incorrect header");
        pictb.close();
        return false;
    }

    int cs_start = sizeof(quint32);                // pos of checksum word
    int data_start = cs_start + sizeof(quint16);
    quint16 cs,ccs;
    QByteArray buf = pictb.buffer();        // pointer to data

    s >> cs;                                // read checksum
    ccs = (quint16) qChecksum(buf.constData() + data_start, buf.size() - data_start);
    if (ccs != cs) {
        qWarning("QPicturePaintEngine::checkFormat: Invalid checksum %x, %x expected",
                  ccs, cs);
        pictb.close();
        return false;
    }

    quint16 major, minor;
    s >> major >> minor;                        // read version number
    if (major > mfhdr_maj) {                // new, incompatible version
        qWarning("QPicturePaintEngine::checkFormat: Incompatible version %d.%d",
                  major, minor);
        pictb.close();
        return false;
    }
    s.setVersion(major != 4 ? major : 3);

    quint8  c, clen;
    s >> c >> clen;
    if (c == QPicturePrivate::PdcBegin) {
        if (!(major >= 1 && major <= 3)) {
            qint32 l, t, w, h;
            s >> l >> t >> w >> h;
            brect = QRect(l, t, w, h);
        }
    } else {
        qWarning("QPicturePaintEngine::checkFormat: Format error");
        pictb.close();
        return false;
    }
    pictb.close();

    formatOk = true;                        // picture seems to be ok
    formatMajor = major;
    formatMinor = minor;
    return true;
}

/*! \internal */
QPaintEngine *QPicture::paintEngine() const
{
    if (!d_func()->paintEngine)
        const_cast<QPicture*>(this)->d_func()->paintEngine.reset(new QPicturePaintEngine);
    return d_func()->paintEngine.data();
}

/*****************************************************************************
  QPicture stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
    \relates QPicture

    Writes picture \a r to the stream \a s and returns a reference to
    the stream.
*/

QDataStream &operator<<(QDataStream &s, const QPicture &r)
{
    quint32 size = r.d_func()->pictb.buffer().size();
    s << size;
    // null picture ?
    if (size == 0)
        return s;
    // just write the whole buffer to the stream
    s.writeRawData (r.d_func()->pictb.buffer(), r.d_func()->pictb.buffer().size());
    return s;
}

/*!
    \relates QPicture

    Reads a picture from the stream \a s into picture \a r and returns
    a reference to the stream.
*/

QDataStream &operator>>(QDataStream &s, QPicture &r)
{
    QDataStream sr;

    // "init"; this code is similar to the beginning of QPicture::cmd()
    sr.setDevice(&r.d_func()->pictb);
    sr.setVersion(r.d_func()->formatMajor);
    quint32 len;
    s >> len;
    QByteArray data;
    if (len > 0) {
        data.resize(len);
        s.readRawData(data.data(), len);
    }

    r.d_func()->pictb.setData(data);
    r.d_func()->resetFormat();
    return s;
}
#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE

#endif // QT_NO_PICTURE

/*!
    \typedef QPicture::DataPtr
    \internal
*/

/*!
    \fn DataPtr &QPicture::data_ptr()
    \internal
*/
