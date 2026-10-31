"""Audio and video files: format, duration, codecs, resolution and tags, read by
Qt Multimedia (FFmpeg) without playing anything."""

from PySide6.QtCore import Qt, QDateTime, QLoggingCategory, QUrl
from PySide6.QtGui import QPixmap
from PySide6.QtMultimedia import QMediaMetaData, QMediaPlayer
from PySide6.QtWidgets import QLabel, QTreeWidget, QTreeWidgetItem, QVBoxLayout, QWidget

from .base import Previewer

QLoggingCategory.setFilterRules("qt.multimedia.ffmpeg*=false")  # else FFmpeg dumps every file to stderr

Key = QMediaMetaData.Key
MEDIA_MIMES = ("audio/", "video/", "application/ogg")
IMAGE_KEYS = {Key.ThumbnailImage, Key.CoverArtImage}
STREAM_KEYS = {Key.AudioBitRate, Key.AudioCodec, Key.VideoBitRate, Key.VideoCodec, Key.VideoFrameRate,
               Key.Resolution, Key.Orientation, Key.HasHdrContent}
COVER_SIZE = 200


def format_duration(ms):
    seconds, ms = divmod(int(ms), 1000)
    hours, seconds = divmod(seconds, 3600)
    minutes, seconds = divmod(seconds, 60)
    text = "%d:%02d:%02d" % (hours, minutes, seconds) if hours else "%d:%02d" % (minutes, seconds)
    return text + ".%03d" % ms


def format_bitrate(bps):
    return "%d kbps" % round(bps / 1000) if bps > 0 else ""


def value_text(metaData, key):
    """Display text for one metadata value, "" if unknown. value() can't convert the enum
    types (FileFormat, codecs, ...), so it is only used for numbers, sizes and dates."""
    if key in (Key.Duration, Key.AudioBitRate, Key.VideoBitRate, Key.VideoFrameRate,
               Key.Resolution, Key.HasHdrContent, Key.Date):
        value = metaData.value(key)
        if value is None:
            return ""
    if key == Key.Duration:
        return format_duration(value) if value > 0 else ""
    if key in (Key.AudioBitRate, Key.VideoBitRate):
        return format_bitrate(value)
    if key == Key.VideoFrameRate:
        return ("%.3f" % value).rstrip("0").rstrip(".") + " fps" if value > 0 else ""
    if key == Key.Resolution:
        return "%d x %d" % (value.width(), value.height()) if value.isValid() else ""
    if key == Key.HasHdrContent:
        return "Yes" if value else ""
    if key == Key.Date and isinstance(value, QDateTime):
        return value.toString("yyyy-MM-dd HH:mm:ss")
    text = metaData.stringValue(key)
    return "" if text in ("None", "Default") else text  # no rotation; undetermined language


def metadata_rows(metaData, skip):
    for key in sorted(metaData.keys(), key=lambda k: k.value):  # Title, Author, ..., as Qt lists them
        if key not in skip and key not in IMAGE_KEYS and (text := value_text(metaData, key)):
            yield QMediaMetaData.metaDataKeyToString(key), text


class MediaView(QWidget):
    """Loads a file into a QMediaPlayer that is never played, and lists its metadata."""

    def __init__(self):
        super().__init__()
        self.cover = QLabel()
        self.cover.setAlignment(Qt.AlignCenter)
        self.tree = QTreeWidget()
        self.tree.setHeaderLabels(("Property", "Value"))
        self.tree.setRootIsDecorated(False)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.cover)
        layout.addWidget(self.tree, 1)

        self.player = QMediaPlayer(self)
        self.player.mediaStatusChanged.connect(self.onStatus)
        self.player.errorOccurred.connect(lambda error, message: self.finish(None, message or error.name))
        self.file = None    # FileInfo being loaded
        self.onDone = None  # onDone(fields, error), when loading finishes after load() returns
        self.result = None
        self.clear()

    def clear(self):
        self.file = self.onDone = self.result = None
        self.player.setSource(QUrl())  # also releases the file
        self.tree.clear()
        self.cover.clear()
        self.cover.hide()

    def load(self, f, onDone):
        """Start loading FileInfo f; returns (fields, error) if that finished at once, else
        None and calls onDone(fields, error) later."""
        self.file = f
        self.player.setSource(QUrl.fromLocalFile(f.path))
        if self.result is None:
            self.onDone = onDone
        return self.result

    def onStatus(self, status):
        if status == QMediaPlayer.LoadedMedia:
            try:
                self.finish(self.showMetaData(), None)
            except Exception as e:  # unexpected metadata must not leave it "loading..."
                self.finish(None, str(e))
        elif status == QMediaPlayer.InvalidMedia:
            self.finish(None, self.player.errorString() or "Can't open this file")

    def finish(self, fields, error):
        if self.file is None or self.result is not None:  # not loading, or already reported
            return
        self.result = fields, error
        self.player.setSource(QUrl())  # all read: don't keep the file open
        onDone, self.onDone = self.onDone, None
        if onDone:
            onDone(fields, error)

    def addSection(self, title, rows):
        if not rows:
            return
        section = QTreeWidgetItem(self.tree, [title])
        section.setFirstColumnSpanned(True)
        font = section.font(0)
        font.setBold(True)
        section.setFont(0, font)
        for name, text in rows:
            item = QTreeWidgetItem(section, [name, text])
            item.setToolTip(1, text)
        section.setExpanded(True)

    def showMetaData(self):
        """Fill the view; returns the info fields."""
        f, player = self.file, self.player
        metaData = player.metaData()
        tracks = [("Video", t) for t in player.videoTracks()] + [("Audio", t) for t in player.audioTracks()] \
            + [("Subtitles", t) for t in player.subtitleTracks()]
        duration = player.duration()

        general = [("Type", f.description)] if f.description else []
        general += metadata_rows(metaData, set(Key) - {Key.FileFormat, Key.Duration})
        if duration > 0:
            general.append(("Bit rate (overall)", format_bitrate(f.size * 8000 / duration)))
        general += metadata_rows(metaData, {Key.FileFormat, Key.Duration} | (STREAM_KEYS if tracks else set()))
        self.addSection("General", general)
        counts = {}
        for kind, track in tracks:
            counts[kind] = counts.get(kind, 0) + 1
            same = sum(k == kind for k, _ in tracks)
            self.addSection(kind + (" %d" % counts[kind] if same > 1 else ""),
                            list(metadata_rows(track, {Key.Duration})))
        self.tree.resizeColumnToContents(0)

        image = metaData.value(Key.CoverArtImage) or metaData.value(Key.ThumbnailImage)
        if image is not None and not image.isNull():
            pixmap = QPixmap.fromImage(image)
            if pixmap.width() > COVER_SIZE or pixmap.height() > COVER_SIZE:
                pixmap = pixmap.scaled(COVER_SIZE, COVER_SIZE, Qt.KeepAspectRatio, Qt.SmoothTransformation)
            self.cover.setPixmap(pixmap)
            self.cover.show()

        fields = [value_text(metaData, Key.FileFormat) or "media"]
        if duration > 0:
            fields.append(format_duration(duration))
        if player.hasVideo():
            fields.append(value_text(metaData, Key.Resolution) or "video")
        return [field for field in fields if field]


class MediaPreviewer(Previewer):
    name = "Media"

    def match(self, f):
        return f.mime.startswith(MEDIA_MIMES)

    def show(self, f, panel):
        def done(fields, error):
            if error:
                panel.clearViews("%s\nError: %s" % (f.path, error))
            else:
                panel.setInfo(f, fields)

        result = panel.view(MediaView).load(f, done)
        if result is None:
            return ["loading..."]
        fields, error = result
        if error:
            raise ValueError(error)
        return fields
