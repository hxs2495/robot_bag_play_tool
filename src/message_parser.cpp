#include "robot_bag_play_tool/message_parser.hpp"

#include "rosbag2_cpp/typesupport_helpers.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

namespace robot_bag_play_tool
{
namespace
{
constexpr int kMaxDepth = 32;

const auto * introspectionMembers(const rosidl_message_type_support_t * type_support)
{
  if (type_support == nullptr || type_support->data == nullptr) {
    return static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(nullptr);
  }
  return static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(type_support->data);
}

}  // namespace

ParsedMessageNode MessageParser::parseType(const std::string & topic_type)
{
  const auto * type_support = getTypeSupport(topic_type);
  return parseMessageType(type_support, QString::fromStdString(topic_type), 0);
}

const rosidl_message_type_support_t * MessageParser::getTypeSupport(const std::string & topic_type)
{
  const auto found = type_support_cache_.find(topic_type);
  if (found != type_support_cache_.end()) {
    return found->second.handle;
  }

  TypeSupportEntry entry;
  entry.library = rosbag2_cpp::get_typesupport_library(
    topic_type, rosidl_typesupport_introspection_cpp::typesupport_identifier);
  entry.handle = rosbag2_cpp::get_typesupport_handle(
    topic_type, rosidl_typesupport_introspection_cpp::typesupport_identifier, entry.library);
  const auto inserted = type_support_cache_.emplace(topic_type, entry);
  return inserted.first->second.handle;
}

ParsedMessageNode MessageParser::parseMessageType(
  const rosidl_message_type_support_t * type_support,
  const QString & name,
  int depth) const
{
  ParsedMessageNode node;
  node.name = name;
  node.type = messageTypeName(type_support);

  const auto * members = introspectionMembers(type_support);
  if (members == nullptr) {
    node.value = "无法读取 introspection 类型信息";
    return node;
  }

  node.value = QString("%1 个字段").arg(members->member_count_);
  if (depth >= kMaxDepth) {
    node.value += "（已达到嵌套深度上限）";
    return node;
  }

  for (uint32_t i = 0; i < members->member_count_; ++i) {
    node.children.push_back(parseFieldType(&members->members_[i], depth + 1));
  }
  return node;
}

ParsedMessageNode MessageParser::parseFieldType(const void * message_member, int depth) const
{
  const auto & member =
    *static_cast<const rosidl_typesupport_introspection_cpp::MessageMember *>(message_member);

  ParsedMessageNode node;
  node.name = QString::fromUtf8(member.name_);
  using namespace rosidl_typesupport_introspection_cpp;
  QString base_type = member.type_id_ == ROS_TYPE_MESSAGE ?
    messageTypeName(member.members_) : typeName(member.type_id_);
  if ((member.type_id_ == ROS_TYPE_STRING || member.type_id_ == ROS_TYPE_WSTRING) &&
    member.string_upper_bound_ > 0)
  {
    base_type += QString("<=%1").arg(member.string_upper_bound_);
  }

  node.type = base_type;
  if (member.is_array_) {
    if (member.is_upper_bound_) {
      node.type += QString("[<=%1]").arg(member.array_size_);
      node.value = QString("有界序列，最多 %1 项").arg(member.array_size_);
    } else if (member.array_size_ > 0) {
      node.type += QString("[%1]").arg(member.array_size_);
      node.value = QString("固定数组，%1 项").arg(member.array_size_);
    } else {
      node.type += "[]";
      node.value = "无界序列";
    }
  } else if (member.string_upper_bound_ > 0) {
    node.value = QString("最大长度 %1").arg(member.string_upper_bound_);
  } else if (member.type_id_ == ROS_TYPE_MESSAGE) {
    node.value = "嵌套消息";
  }

  if (member.type_id_ == ROS_TYPE_MESSAGE && member.members_ != nullptr) {
    const auto nested = parseMessageType(member.members_, node.name, depth);
    node.children = nested.children;
    if (depth >= kMaxDepth) {
      node.value += "（已达到嵌套深度上限）";
    }
  }
  return node;
}

QString MessageParser::messageTypeName(
  const rosidl_message_type_support_t * type_support) const
{
  const auto * members = introspectionMembers(type_support);
  if (members == nullptr) {
    return "message";
  }

  QString message_namespace = QString::fromUtf8(members->message_namespace_);
  message_namespace.replace("::", "/");
  if (!message_namespace.isEmpty()) {
    message_namespace += "/";
  }
  return message_namespace + QString::fromUtf8(members->message_name_);
}

QString MessageParser::typeName(uint8_t type_id) const
{
  using namespace rosidl_typesupport_introspection_cpp;
  switch (type_id) {
    case ROS_TYPE_FLOAT:
      return "float32";
    case ROS_TYPE_DOUBLE:
      return "float64";
    case ROS_TYPE_LONG_DOUBLE:
      return "long double";
    case ROS_TYPE_CHAR:
      return "char";
    case ROS_TYPE_WCHAR:
      return "wchar";
    case ROS_TYPE_BOOLEAN:
      return "bool";
    case ROS_TYPE_OCTET:
      return "byte";
    case ROS_TYPE_UINT8:
      return "uint8";
    case ROS_TYPE_INT8:
      return "int8";
    case ROS_TYPE_UINT16:
      return "uint16";
    case ROS_TYPE_INT16:
      return "int16";
    case ROS_TYPE_UINT32:
      return "uint32";
    case ROS_TYPE_INT32:
      return "int32";
    case ROS_TYPE_UINT64:
      return "uint64";
    case ROS_TYPE_INT64:
      return "int64";
    case ROS_TYPE_STRING:
      return "string";
    case ROS_TYPE_WSTRING:
      return "wstring";
    case ROS_TYPE_MESSAGE:
      return "message";
    default:
      return "unknown";
  }
}

}  // namespace robot_bag_play_tool
