#ifndef RELAY_ENDPOINT_HPP
#define RELAY_ENDPOINT_HPP

#include <optional>
#include <string>

namespace relay {

/*
 * 打开路径；无值 / "-" → *out_fd = std_fd，*out_owns = false。
 * 成功 true；失败 false。
 */
bool open_endpoint(const std::optional<std::string> &path, int std_fd, int flags,
                   int *out_fd, bool *out_owns);

} // namespace relay

#endif
