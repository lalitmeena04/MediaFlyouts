#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QTimer>

#include "playbackcontroller.h"
#include "trackinfo.h"

// QML-facing now-playing model; fed by a media provider and forwards transport commands to a controller.
class NowPlaying : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY changed)
    Q_PROPERTY(QString artist READ artist NOTIFY changed)
    Q_PROPERTY(QString elapsed READ elapsed NOTIFY changed)
    Q_PROPERTY(QString duration READ duration NOTIFY changed)
    Q_PROPERTY(double progress READ progress NOTIFY changed)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY changed)
    Q_PROPERTY(bool hasMedia READ hasMedia NOTIFY changed)

public:
    explicit NowPlaying(QObject* parent = nullptr);

    void setController(mf::IPlaybackController* controller);

    QString title() const;
    QString artist() const;
    QString elapsed() const;
    QString duration() const;
    double progress() const;
    bool isPlaying() const;
    bool hasMedia() const;

    Q_INVOKABLE void playPause();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();

public slots:
    void applyTrack(const mf::TrackInfo& track);

signals:
    void changed();
    void trackUpdated();

private:
    qint64 currentPositionMs() const;

    mf::TrackInfo m_track;
    QElapsedTimer m_sinceUpdate;
    QTimer m_timer;
    mf::IPlaybackController* m_controller = nullptr;
};
