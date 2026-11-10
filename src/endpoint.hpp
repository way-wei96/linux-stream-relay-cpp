#ifndef RELAY_ENDPOINT_HPP
#define RELAY_ENDPOINT_HPP

#include "unique_fd.hpp"

#include <optional>
#include <string>

namespace relay {

/*
 * 打开路径；无值 / "-" → 包装 std_fd（owns=false）。
 * 成功有值；失败 nullopt。
 */
std::optional<UniqueFd> open_endpoint(const std::optional<std::string> &path,
                                      int std_fd, int flags);

} // namespace relay

#endif
