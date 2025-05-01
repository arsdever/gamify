#pragma once

namespace core
{
template <typename T, typename U, typename V>
bool rect_contains(const glm::vec<2, T>& rectlb,
                   const glm::vec<2, U>& rectrt,
                   const glm::vec<2, V>& point)
{
    return point.x >= rectlb.x && point.x <= rectrt.x && point.y >= rectlb.y &&
           point.y <= rectrt.y;
}
} // namespace core
