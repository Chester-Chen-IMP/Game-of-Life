#include "getWidth.hpp"

// 将 UTF-8 序列解码为一个 Unicode 码点，返回解码后的码点以及消耗的字节数
std::pair<uint32_t, int> decodeUtf8(const std::string& str, size_t pos) {
  if (pos >= str.size()) return {0, 0};

  uint8_t c = static_cast<uint8_t>(str[pos]);

  // 1-byte sequence (ASCII)
  if ((c & 0x80) == 0) return {c, 1};

  // 2-byte sequence
  if ((c & 0xE0) == 0xC0) {
    if (pos + 1 >= str.size()) return {0, 0};
    uint32_t cp =
      ((c & 0x1F) << 6) | (static_cast<uint8_t>(str[pos + 1]) & 0x3F);
    return {cp, 2};
  }

  // 3-byte sequence
  if ((c & 0xF0) == 0xE0) {
    if (pos + 2 >= str.size()) return {0, 0};
    uint32_t cp = ((c & 0x0F) << 12)
                  | ((static_cast<uint8_t>(str[pos + 1]) & 0x3F) << 6)
                  | (static_cast<uint8_t>(str[pos + 2]) & 0x3F);
    return {cp, 3};
  }

  // 4-byte sequence
  if ((c & 0xF8) == 0xF0) {
    if (pos + 3 >= str.size()) return {0, 0};
    uint32_t cp = ((c & 0x07) << 18)
                  | ((static_cast<uint8_t>(str[pos + 1]) & 0x3F) << 12)
                  | ((static_cast<uint8_t>(str[pos + 2]) & 0x3F) << 6)
                  | (static_cast<uint8_t>(str[pos + 3]) & 0x3F);
    return {cp, 4};
  }

  // 非法字节，跳过
  return {0, 1};
}

// 判断一个 Unicode 码点是否属于“宽字符”（占2列）
bool isWchar(uint32_t cp) {
  // 控制字符、DEL 等宽度为 0，这里全部忽略
  if (cp < 0x20 || cp == 0x7F) return false;

  // 基本拉丁、拉丁补充等通常占 1 列，直接排除
  if (cp < 0x1100) return false;

  // 下列范围全部视为 2 列宽
  // 谚文字母
  if (cp >= 0x1100 && cp <= 0x115F) return true;
  // 尖括号
  if (cp == 0x2329 || cp == 0x232A) return true;
  // CJK 部首补充、康熙部首、表意文字描述符、CJK 符号和标点、平假名、片假名
  if (cp >= 0x2E80 && cp <= 0x33BF) return true;
  // CJK 统一表意文字扩展 A
  if (cp >= 0x3400 && cp <= 0x4DBF) return true;
  // CJK 统一表意文字
  if (cp >= 0x4E00 && cp <= 0x9FFF) return true;
  // 彝文
  if (cp >= 0xA000 && cp <= 0xA4CF) return true;
  // 谚文音节
  if (cp >= 0xAC00 && cp <= 0xD7A3) return true;
  // CJK 兼容象形文字
  if (cp >= 0xF900 && cp <= 0xFAFF) return true;
  // 竖排标点
  if (cp >= 0xFE10 && cp <= 0xFE19) return true;
  // CJK 兼容形式
  if (cp >= 0xFE30 && cp <= 0xFE6F) return true;
  // 全角 ASCII、全角符号
  if (cp >= 0xFF01 && cp <= 0xFF60) return true;
  if (cp >= 0xFFE0 && cp <= 0xFFE6) return true;

  // 常见 Emoji 及符号块（大多属于“模糊宽度”，终端中通常显示为 2 列）
  if (cp >= 0x1F000 && cp <= 0x1FAFF) return true;

  // CJK 扩展 B~H 等补充平面（简化为 0x20000 以上全部按 2 列处理）
  if (cp >= 0x20000) return true;

  // 默认返回 1 列
  return false;
}

// 计算一个 UTF-8 字符串的显示宽度
int getWidth(const std::string& str) {
  int width  = 0;
  size_t pos = 0;

  while (pos < str.size()) {
    auto [cp, len] = decodeUtf8(str, pos);
    if (len == 0) break;  // 解码失败，退出

    if (isWchar(cp))
      width += 2;
    else if (cp >= 0x20 && cp != 0x7F)  // 可打印 ASCII 占 1 列，控制字符忽略
      width += 1;
    // 其余控制字符宽度为 0，不累加

    pos += len;
  }
  return width;
}
