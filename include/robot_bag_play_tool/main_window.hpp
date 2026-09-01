#ifndef ROBOT_BAG_PLAY_TOOL__MAIN_WINDOW_HPP_
#define ROBOT_BAG_PLAY_TOOL__MAIN_WINDOW_HPP_

#include <memory>
#include <unordered_map>

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTreeView>

#include "robot_bag_play_tool/bag_player.hpp"
#include "robot_bag_play_tool/message_parser.hpp"
#include "robot_bag_play_tool/timeline_widget.hpp"

class QCloseEvent;
class QComboBox;
class QSplitter;

namespace robot_bag_play_tool
{

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(std::shared_ptr<BagPlayer> bag_player, QWidget * parent = nullptr);
  ~MainWindow() override = default;

protected:
  void closeEvent(QCloseEvent * event) override;

private slots:
  void openBag();
  void closeBag();
  void refreshTopics();
  void resetUi();
  void updateTopicStatus(
    const QString & topic_name,
    const QString & status,
    uint64_t published_count,
    double frequency_hz);
  void showTopicType(int row, int column);
  void updateClock(int64_t timestamp_ns);
  void updateProgress(double progress);
  void seekTo(int64_t timestamp_ns);

private:
  void buildUi();
  void restoreUiState();
  void saveUiState() const;
  void populateTree(const ParsedMessageNode & node);
  void addTreeNode(QStandardItem * parent, const ParsedMessageNode & node);
  QString formatDuration(int64_t duration_ns) const;
  QString formatBytes(uint64_t bytes) const;
  int rowForTopic(const QString & topic_name) const;

  std::shared_ptr<BagPlayer> bag_player_;
  MessageParser parser_;
  QLabel * bag_path_label_ = nullptr;
  QLabel * bag_info_label_ = nullptr;
  QLabel * clock_label_ = nullptr;
  QLabel * status_label_ = nullptr;
  TimelineWidget * timeline_widget_ = nullptr;
  QTableWidget * topic_table_ = nullptr;
  QTreeView * tree_view_ = nullptr;
  QLabel * selected_topic_label_ = nullptr;
  QProgressBar * progress_bar_ = nullptr;
  QStandardItemModel * tree_model_ = nullptr;
  QComboBox * speed_box_ = nullptr;
  QSplitter * content_splitter_ = nullptr;
  std::unordered_map<std::string, int> topic_rows_;
};

}  // namespace robot_bag_play_tool

#endif  // ROBOT_BAG_PLAY_TOOL__MAIN_WINDOW_HPP_
