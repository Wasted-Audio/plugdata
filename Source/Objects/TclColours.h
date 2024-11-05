/*
 // Copyright (c) 2021-2022 Timothy Schoen and Pierre Guillot
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#pragma once

UnorderedMap<String, Colour> tclColours {
    { "alice blue", Colour(240, 248, 255) },
    { "AliceBlue", Colour(240, 248, 255) },
    { "antique white", Colour(250, 235, 215) },
    { "AntiqueWhite", Colour(250, 235, 215) },
    { "AntiqueWhite1", Colour(255, 239, 219) },
    { "AntiqueWhite2", Colour(238, 223, 204) },
    { "AntiqueWhite3", Colour(205, 192, 176) },
    { "AntiqueWhite4", Colour(139, 131, 120) },
    { "agua", Colour(0, 255, 255) },
    { "aquamarine", Colour(127, 255, 212) },
    { "aquamarine1", Colour(127, 255, 212) },
    { "aquamarine2", Colour(118, 238, 198) },
    { "aquamarine3", Colour(102, 205, 170) },
    { "aquamarine4", Colour(69, 139, 116) },
    { "azure", Colour(240, 255, 255) },
    { "azure1", Colour(240, 255, 255) },
    { "azure2", Colour(224, 238, 238) },
    { "azure3", Colour(193, 205, 205) },
    { "azure4", Colour(131, 139, 139) },
    { "beige", Colour(245, 245, 220) },
    { "bisque", Colour(255, 228, 196) },
    { "bisque1", Colour(255, 228, 196) },
    { "bisque2", Colour(238, 213, 183) },
    { "bisque3", Colour(205, 183, 158) },
    { "bisque4", Colour(139, 125, 107) },
    { "black", Colour(0, 0, 0) },
    { "blanched almond", Colour(255, 235, 205) },
    { "BlanchedAlmond", Colour(255, 235, 205) },
    { "blue", Colour(0, 0, 255) },
    { "blue violet", Colour(138, 43, 226) },
    { "blue1", Colour(0, 0, 255) },
    { "blue2", Colour(0, 0, 238) },
    { "blue3", Colour(0, 0, 205) },
    { "blue4", Colour(0, 0, 139) },
    { "BlueViolet", Colour(138, 43, 226) },
    { "brown", Colour(165, 42, 42) },
    { "brown1", Colour(255, 64, 64) },
    { "brown2", Colour(238, 59, 59) },
    { "brown3", Colour(205, 51, 51) },
    { "brown4", Colour(139, 35, 35) },
    { "burlywood", Colour(222, 184, 135) },
    { "burlywood1", Colour(255, 211, 155) },
    { "burlywood2", Colour(238, 197, 145) },
    { "burlywood3", Colour(205, 170, 125) },
    { "burlywood4", Colour(139, 115, 85) },
    { "cadet blue", Colour(95, 158, 160) },
    { "CadetBlue", Colour(95, 158, 160) },
    { "CadetBlue1", Colour(152, 245, 255) },
    { "CadetBlue2", Colour(142, 229, 238) },
    { "CadetBlue3", Colour(122, 197, 205) },
    { "CadetBlue4", Colour(83, 134, 139) },
    { "chartreuse", Colour(127, 255, 0) },
    { "chartreuse1", Colour(127, 255, 0) },
    { "chartreuse2", Colour(118, 238, 0) },
    { "chartreuse3", Colour(102, 205, 0) },
    { "chartreuse4", Colour(69, 139, 0) },
    { "chocolate", Colour(210, 105, 30) },
    { "chocolate1", Colour(255, 127, 36) },
    { "chocolate2", Colour(238, 118, 33) },
    { "chocolate3", Colour(205, 102, 29) },
    { "chocolate4", Colour(139, 69, 19) },
    { "coral", Colour(255, 127, 80) },
    { "coral1", Colour(255, 114, 86) },
    { "coral2", Colour(238, 106, 80) },
    { "coral3", Colour(205, 91, 69) },
    { "coral4", Colour(139, 62, 47) },
    { "cornflower blue", Colour(100, 149, 237) },
    { "CornflowerBlue", Colour(100, 149, 237) },
    { "cornsilk", Colour(255, 248, 220) },
    { "cornsilk1", Colour(255, 248, 220) },
    { "cornsilk2", Colour(238, 232, 205) },
    { "cornsilk3", Colour(205, 200, 177) },
    { "cornsilk4", Colour(139, 136, 120) },
    { "crymson", Colour(220, 20, 60) },
    { "cyan", Colour(0, 255, 255) },
    { "cyan1", Colour(0, 255, 255) },
    { "cyan2", Colour(0, 238, 238) },
    { "cyan3", Colour(0, 205, 205) },
    { "cyan4", Colour(0, 139, 139) },
    { "dark blue", Colour(0, 0, 139) },
    { "dark cyan", Colour(0, 139, 139) },
    { "dark goldenrod", Colour(184, 134, 11) },
    { "dark gray", Colour(169, 169, 169) },
    { "dark green", Colour(0, 100, 0) },
    { "dark grey", Colour(169, 169, 169) },
    { "dark khaki", Colour(189, 183, 107) },
    { "dark magenta", Colour(139, 0, 139) },
    { "dark olive green", Colour(85, 107, 47) },
    { "dark orange", Colour(255, 140, 0) },
    { "dark orchid", Colour(153, 50, 204) },
    { "dark red", Colour(139, 0, 0) },
    { "dark salmon", Colour(233, 150, 122) },
    { "dark sea green", Colour(143, 188, 143) },
    { "dark slate blue", Colour(72, 61, 139) },
    { "dark slate gray", Colour(47, 79, 79) },
    { "dark slate grey", Colour(47, 79, 79) },
    { "dark turquoise", Colour(0, 206, 209) },
    { "dark violet", Colour(148, 0, 211) },
    { "DarkBlue", Colour(0, 0, 139) },
    { "DarkCyan", Colour(0, 139, 139) },
    { "DarkGoldenrod", Colour(184, 134, 11) },
    { "DarkGoldenrod1", Colour(255, 185, 15) },
    { "DarkGoldenrod2", Colour(238, 173, 14) },
    { "DarkGoldenrod3", Colour(205, 149, 12) },
    { "DarkGoldenrod4", Colour(139, 101, 8) },
    { "DarkGray", Colour(169, 169, 169) },
    { "DarkGreen", Colour(0, 100, 0) },
    { "DarkGrey", Colour(169, 169, 169) },
    { "DarkKhaki", Colour(189, 183, 107) },
    { "DarkMagenta", Colour(139, 0, 139) },
    { "DarkOliveGreen", Colour(85, 107, 47) },
    { "DarkOliveGreen1", Colour(202, 255, 112) },
    { "DarkOliveGreen2", Colour(188, 238, 104) },
    { "DarkOliveGreen3", Colour(162, 205, 90) },
    { "DarkOliveGreen4", Colour(110, 139, 61) },
    { "DarkOrange", Colour(255, 140, 0) },
    { "DarkOrange1", Colour(255, 127, 0) },
    { "DarkOrange2", Colour(238, 118, 0) },
    { "DarkOrange3", Colour(205, 102, 0) },
    { "DarkOrange4", Colour(139, 69, 0) },
    { "DarkOrchid", Colour(153, 50, 204) },
    { "DarkOrchid1", Colour(191, 62, 255) },
    { "DarkOrchid2", Colour(178, 58, 238) },
    { "DarkOrchid3", Colour(154, 50, 205) },
    { "DarkOrchid4", Colour(104, 34, 139) },
    { "DarkRed", Colour(139, 0, 0) },
    { "DarkSalmon", Colour(233, 150, 122) },
    { "DarkSeaGreen", Colour(143, 188, 143) },
    { "DarkSeaGreen1", Colour(193, 255, 193) },
    { "DarkSeaGreen2", Colour(180, 238, 180) },
    { "DarkSeaGreen3", Colour(155, 205, 155) },
    { "DarkSeaGreen4", Colour(105, 139, 105) },
    { "DarkSlateBlue", Colour(72, 61, 139) },
    { "DarkSlateGray", Colour(47, 79, 79) },
    { "DarkSlateGray1", Colour(151, 255, 255) },
    { "DarkSlateGray2", Colour(141, 238, 238) },
    { "DarkSlateGray3", Colour(121, 205, 205) },
    { "DarkSlateGray4", Colour(82, 139, 139) },
    { "DarkSlateGrey", Colour(47, 79, 79) },
    { "DarkTurquoise", Colour(0, 206, 209) },
    { "DarkViolet", Colour(148, 0, 211) },
    { "deep pink", Colour(255, 20, 147) },
    { "deep sky blue", Colour(0, 191, 255) },
    { "DeepPink", Colour(255, 20, 147) },
    { "DeepPink1", Colour(255, 20, 147) },
    { "DeepPink2", Colour(238, 18, 137) },
    { "DeepPink3", Colour(205, 16, 118) },
    { "DeepPink4", Colour(139, 10, 80) },
    { "DeepSkyBlue", Colour(0, 191, 255) },
    { "DeepSkyBlue1", Colour(0, 191, 255) },
    { "DeepSkyBlue2", Colour(0, 178, 238) },
    { "DeepSkyBlue3", Colour(0, 154, 205) },
    { "DeepSkyBlue4", Colour(0, 104, 139) },
    { "dim gray", Colour(105, 105, 105) },
    { "dim grey", Colour(105, 105, 105) },
    { "DimGray", Colour(105, 105, 105) },
    { "DimGrey", Colour(105, 105, 105) },
    { "dodger blue", Colour(30, 144, 255) },
    { "DodgerBlue", Colour(30, 144, 255) },
    { "DodgerBlue1", Colour(30, 144, 255) },
    { "DodgerBlue2", Colour(28, 134, 238) },
    { "DodgerBlue3", Colour(24, 116, 205) },
    { "DodgerBlue4", Colour(16, 78, 139) },
    { "firebrick", Colour(178, 34, 34) },
    { "firebrick1", Colour(255, 48, 48) },
    { "firebrick2", Colour(238, 44, 44) },
    { "firebrick3", Colour(205, 38, 38) },
    { "firebrick4", Colour(139, 26, 26) },
    { "floral white", Colour(255, 250, 240) },
    { "FloralWhite", Colour(255, 250, 240) },
    { "forest green", Colour(34, 139, 34) },
    { "ForestGreen", Colour(34, 139, 34) },
    { "fuchsia", Colour(255, 0, 255) },
    { "gainsboro", Colour(220, 220, 220) },
    { "ghost white", Colour(248, 248, 255) },
    { "GhostWhite", Colour(248, 248, 255) },
    { "gold", Colour(255, 215, 0) },
    { "gold1", Colour(255, 215, 0) },
    { "gold2", Colour(238, 201, 0) },
    { "gold3", Colour(205, 173, 0) },
    { "gold4", Colour(139, 117, 0) },
    { "goldenrod", Colour(218, 165, 32) },
    { "goldenrod1", Colour(255, 193, 37) },
    { "goldenrod2", Colour(238, 180, 34) },
    { "goldenrod3", Colour(205, 155, 29) },
    { "goldenrod4", Colour(139, 105, 20) },
    { "gray", Colour(128, 128, 128) },
    { "gray0", Colour(0, 0, 0) },
    { "gray1", Colour(3, 3, 3) },
    { "gray2", Colour(5, 5, 5) },
    { "gray3", Colour(8, 8, 8) },
    { "gray4", Colour(10, 10, 10) },
    { "gray5", Colour(13, 13, 13) },
    { "gray6", Colour(15, 15, 15) },
    { "gray7", Colour(18, 18, 18) },
    { "gray8", Colour(20, 20, 20) },
    { "gray9", Colour(23, 23, 23) },
    { "gray10", Colour(26, 26, 26) },
    { "gray11", Colour(28, 28, 28) },
    { "gray12", Colour(31, 31, 31) },
    { "gray13", Colour(33, 33, 33) },
    { "gray14", Colour(36, 36, 36) },
    { "gray15", Colour(38, 38, 38) },
    { "gray16", Colour(41, 41, 41) },
    { "gray17", Colour(43, 43, 43) },
    { "gray18", Colour(46, 46, 46) },
    { "gray19", Colour(48, 48, 48) },
    { "gray20", Colour(51, 51, 51) },
    { "gray21", Colour(54, 54, 54) },
    { "gray22", Colour(56, 56, 56) },
    { "gray23", Colour(59, 59, 59) },
    { "gray24", Colour(61, 61, 61) },
    { "gray25", Colour(64, 64, 64) },
    { "gray26", Colour(66, 66, 66) },
    { "gray27", Colour(69, 69, 69) },
    { "gray28", Colour(71, 71, 71) },
    { "gray29", Colour(74, 74, 74) },
    { "gray30", Colour(77, 77, 77) },
    { "gray31", Colour(79, 79, 79) },
    { "gray32", Colour(82, 82, 82) },
    { "gray33", Colour(84, 84, 84) },
    { "gray34", Colour(87, 87, 87) },
    { "gray35", Colour(89, 89, 89) },
    { "gray36", Colour(92, 92, 92) },
    { "gray37", Colour(94, 94, 94) },
    { "gray38", Colour(97, 97, 97) },
    { "gray39", Colour(99, 99, 99) },
    { "gray40", Colour(102, 102, 102) },
    { "gray41", Colour(105, 105, 105) },
    { "gray42", Colour(107, 107, 107) },
    { "gray43", Colour(110, 110, 110) },
    { "gray44", Colour(112, 112, 112) },
    { "gray45", Colour(115, 115, 115) },
    { "gray46", Colour(117, 117, 117) },
    { "gray47", Colour(120, 120, 120) },
    { "gray48", Colour(122, 122, 122) },
    { "gray49", Colour(125, 125, 125) },
    { "gray50", Colour(127, 127, 127) },
    { "gray51", Colour(130, 130, 130) },
    { "gray52", Colour(133, 133, 133) },
    { "gray53", Colour(135, 135, 135) },
    { "gray54", Colour(138, 138, 138) },
    { "gray55", Colour(140, 140, 140) },
    { "gray56", Colour(143, 143, 143) },
    { "gray57", Colour(145, 145, 145) },
    { "gray58", Colour(148, 148, 148) },
    { "gray59", Colour(150, 150, 150) },
    { "gray60", Colour(153, 153, 153) },
    { "gray61", Colour(156, 156, 156) },
    { "gray62", Colour(158, 158, 158) },
    { "gray63", Colour(161, 161, 161) },
    { "gray64", Colour(163, 163, 163) },
    { "gray65", Colour(166, 166, 166) },
    { "gray66", Colour(168, 168, 168) },
    { "gray67", Colour(171, 171, 171) },
    { "gray68", Colour(173, 173, 173) },
    { "gray69", Colour(176, 176, 176) },
    { "gray70", Colour(179, 179, 179) },
    { "gray71", Colour(181, 181, 181) },
    { "gray72", Colour(184, 184, 184) },
    { "gray73", Colour(186, 186, 186) },
    { "gray74", Colour(189, 189, 189) },
    { "gray75", Colour(191, 191, 191) },
    { "gray76", Colour(194, 194, 194) },
    { "gray77", Colour(196, 196, 196) },
    { "gray78", Colour(199, 199, 199) },
    { "gray79", Colour(201, 201, 201) },
    { "gray80", Colour(204, 204, 204) },
    { "gray81", Colour(207, 207, 207) },
    { "gray82", Colour(209, 209, 209) },
    { "gray83", Colour(212, 212, 212) },
    { "gray84", Colour(214, 214, 214) },
    { "gray85", Colour(217, 217, 217) },
    { "gray86", Colour(219, 219, 219) },
    { "gray87", Colour(222, 222, 222) },
    { "gray88", Colour(224, 224, 224) },
    { "gray89", Colour(227, 227, 227) },
    { "gray90", Colour(229, 229, 229) },
    { "gray91", Colour(232, 232, 232) },
    { "gray92", Colour(235, 235, 235) },
    { "gray93", Colour(237, 237, 237) },
    { "gray94", Colour(240, 240, 240) },
    { "gray95", Colour(242, 242, 242) },
    { "gray96", Colour(245, 245, 245) },
    { "gray97", Colour(247, 247, 247) },
    { "gray98", Colour(250, 250, 250) },
    { "gray99", Colour(252, 252, 252) },
    { "gray100", Colour(255, 255, 255) },
    { "green", Colour(0, 128, 0) },
    { "green yellow", Colour(173, 255, 47) },
    { "green1", Colour(0, 255, 0) },
    { "green2", Colour(0, 238, 0) },
    { "green3", Colour(0, 205, 0) },
    { "green4", Colour(0, 139, 0) },
    { "GreenYellow", Colour(173, 255, 47) },
    { "grey", Colour(128, 128, 128) },
    { "grey0", Colour(0, 0, 0) },
    { "grey1", Colour(3, 3, 3) },
    { "grey2", Colour(5, 5, 5) },
    { "grey3", Colour(8, 8, 8) },
    { "grey4", Colour(10, 10, 10) },
    { "grey5", Colour(13, 13, 13) },
    { "grey6", Colour(15, 15, 15) },
    { "grey7", Colour(18, 18, 18) },
    { "grey8", Colour(20, 20, 20) },
    { "grey9", Colour(23, 23, 23) },
    { "grey10", Colour(26, 26, 26) },
    { "grey11", Colour(28, 28, 28) },
    { "grey12", Colour(31, 31, 31) },
    { "grey13", Colour(33, 33, 33) },
    { "grey14", Colour(36, 36, 36) },
    { "grey15", Colour(38, 38, 38) },
    { "grey16", Colour(41, 41, 41) },
    { "grey17", Colour(43, 43, 43) },
    { "grey18", Colour(46, 46, 46) },
    { "grey19", Colour(48, 48, 48) },
    { "grey20", Colour(51, 51, 51) },
    { "grey21", Colour(54, 54, 54) },
    { "grey22", Colour(56, 56, 56) },
    { "grey23", Colour(59, 59, 59) },
    { "grey24", Colour(61, 61, 61) },
    { "grey25", Colour(64, 64, 64) },
    { "grey26", Colour(66, 66, 66) },
    { "grey27", Colour(69, 69, 69) },
    { "grey28", Colour(71, 71, 71) },
    { "grey29", Colour(74, 74, 74) },
    { "grey30", Colour(77, 77, 77) },
    { "grey31", Colour(79, 79, 79) },
    { "grey32", Colour(82, 82, 82) },
    { "grey33", Colour(84, 84, 84) },
    { "grey34", Colour(87, 87, 87) },
    { "grey35", Colour(89, 89, 89) },
    { "grey36", Colour(92, 92, 92) },
    { "grey37", Colour(94, 94, 94) },
    { "grey38", Colour(97, 97, 97) },
    { "grey39", Colour(99, 99, 99) },
    { "grey40", Colour(102, 102, 102) },
    { "grey41", Colour(105, 105, 105) },
    { "grey42", Colour(107, 107, 107) },
    { "grey43", Colour(110, 110, 110) },
    { "grey44", Colour(112, 112, 112) },
    { "grey45", Colour(115, 115, 115) },
    { "grey46", Colour(117, 117, 117) },
    { "grey47", Colour(120, 120, 120) },
    { "grey48", Colour(122, 122, 122) },
    { "grey49", Colour(125, 125, 125) },
    { "grey50", Colour(127, 127, 127) },
    { "grey51", Colour(130, 130, 130) },
    { "grey52", Colour(133, 133, 133) },
    { "grey53", Colour(135, 135, 135) },
    { "grey54", Colour(138, 138, 138) },
    { "grey55", Colour(140, 140, 140) },
    { "grey56", Colour(143, 143, 143) },
    { "grey57", Colour(145, 145, 145) },
    { "grey58", Colour(148, 148, 148) },
    { "grey59", Colour(150, 150, 150) },
    { "grey60", Colour(153, 153, 153) },
    { "grey61", Colour(156, 156, 156) },
    { "grey62", Colour(158, 158, 158) },
    { "grey63", Colour(161, 161, 161) },
    { "grey64", Colour(163, 163, 163) },
    { "grey65", Colour(166, 166, 166) },
    { "grey66", Colour(168, 168, 168) },
    { "grey67", Colour(171, 171, 171) },
    { "grey68", Colour(173, 173, 173) },
    { "grey69", Colour(176, 176, 176) },
    { "grey70", Colour(179, 179, 179) },
    { "grey71", Colour(181, 181, 181) },
    { "grey72", Colour(184, 184, 184) },
    { "grey73", Colour(186, 186, 186) },
    { "grey74", Colour(189, 189, 189) },
    { "grey75", Colour(191, 191, 191) },
    { "grey76", Colour(194, 194, 194) },
    { "grey77", Colour(196, 196, 196) },
    { "grey78", Colour(199, 199, 199) },
    { "grey79", Colour(201, 201, 201) },
    { "grey80", Colour(204, 204, 204) },
    { "grey81", Colour(207, 207, 207) },
    { "grey82", Colour(209, 209, 209) },
    { "grey83", Colour(212, 212, 212) },
    { "grey84", Colour(214, 214, 214) },
    { "grey85", Colour(217, 217, 217) },
    { "grey86", Colour(219, 219, 219) },
    { "grey87", Colour(222, 222, 222) },
    { "grey88", Colour(224, 224, 224) },
    { "grey89", Colour(227, 227, 227) },
    { "grey90", Colour(229, 229, 229) },
    { "grey91", Colour(232, 232, 232) },
    { "grey92", Colour(235, 235, 235) },
    { "grey93", Colour(237, 237, 237) },
    { "grey94", Colour(240, 240, 240) },
    { "grey95", Colour(242, 242, 242) },
    { "grey96", Colour(245, 245, 245) },
    { "grey97", Colour(247, 247, 247) },
    { "grey98", Colour(250, 250, 250) },
    { "grey99", Colour(252, 252, 252) },
    { "grey100", Colour(255, 255, 255) },
    { "honeydew", Colour(240, 255, 240) },
    { "honeydew1", Colour(240, 255, 240) },
    { "honeydew2", Colour(224, 238, 224) },
    { "honeydew3", Colour(193, 205, 193) },
    { "honeydew4", Colour(131, 139, 131) },
    { "hot pink", Colour(255, 105, 180) },
    { "HotPink", Colour(255, 105, 180) },
    { "HotPink1", Colour(255, 110, 180) },
    { "HotPink2", Colour(238, 106, 167) },
    { "HotPink3", Colour(205, 96, 144) },
    { "HotPink4", Colour(139, 58, 98) },
    { "indian red", Colour(205, 92, 92) },
    { "IndianRed", Colour(205, 92, 92) },
    { "IndianRed1", Colour(255, 106, 106) },
    { "IndianRed2", Colour(238, 99, 99) },
    { "IndianRed3", Colour(205, 85, 85) },
    { "IndianRed4", Colour(139, 58, 58) },
    { "indigo", Colour(75, 0, 130) },
    { "ivory", Colour(255, 255, 240) },
    { "ivory1", Colour(255, 255, 240) },
    { "ivory2", Colour(238, 238, 224) },
    { "ivory3", Colour(205, 205, 193) },
    { "ivory4", Colour(139, 139, 131) },
    { "khaki", Colour(240, 230, 140) },
    { "khaki1", Colour(255, 246, 143) },
    { "khaki2", Colour(238, 230, 133) },
    { "khaki3", Colour(205, 198, 115) },
    { "khaki4", Colour(139, 134, 78) },
    { "lavender", Colour(230, 230, 250) },
    { "lavender blush", Colour(255, 240, 245) },
    { "LavenderBlush", Colour(255, 240, 245) },
    { "LavenderBlush1", Colour(255, 240, 245) },
    { "LavenderBlush2", Colour(238, 224, 229) },
    { "LavenderBlush3", Colour(205, 193, 197) },
    { "LavenderBlush4", Colour(139, 131, 134) },
    { "lawn green", Colour(124, 252, 0) },
    { "LawnGreen", Colour(124, 252, 0) },
    { "lemon chiffon", Colour(255, 250, 205) },
    { "LemonChiffon", Colour(255, 250, 205) },
    { "LemonChiffon1", Colour(255, 250, 205) },
    { "LemonChiffon2", Colour(238, 233, 191) },
    { "LemonChiffon3", Colour(205, 201, 165) },
    { "LemonChiffon4", Colour(139, 137, 112) },
    { "light blue", Colour(173, 216, 230) },
    { "light coral", Colour(240, 128, 128) },
    { "light cyan", Colour(224, 255, 255) },
    { "light goldenrod", Colour(238, 221, 130) },
    { "light goldenrod yellow", Colour(250, 250, 210) },
    { "light gray", Colour(211, 211, 211) },
    { "light green", Colour(144, 238, 144) },
    { "light grey", Colour(211, 211, 211) },
    { "light pink", Colour(255, 182, 193) },
    { "light salmon", Colour(255, 160, 122) },
    { "light sea green", Colour(32, 178, 170) },
    { "light sky blue", Colour(135, 206, 250) },
    { "light slate blue", Colour(132, 112, 255) },
    { "light slate gray", Colour(119, 136, 153) },
    { "light slate grey", Colour(119, 136, 153) },
    { "light steel blue", Colour(176, 196, 222) },
    { "light yellow", Colour(255, 255, 224) },
    { "LightBlue", Colour(173, 216, 230) },
    { "LightBlue1", Colour(191, 239, 255) },
    { "LightBlue2", Colour(178, 223, 238) },
    { "LightBlue3", Colour(154, 192, 205) },
    { "LightBlue4", Colour(104, 131, 139) },
    { "LightCoral", Colour(240, 128, 128) },
    { "LightCyan", Colour(224, 255, 255) },
    { "LightCyan1", Colour(224, 255, 255) },
    { "LightCyan2", Colour(209, 238, 238) },
    { "LightCyan3", Colour(180, 205, 205) },
    { "LightCyan4", Colour(122, 139, 139) },
    { "LightGoldenrod", Colour(238, 221, 130) },
    { "LightGoldenrod1", Colour(255, 236, 139) },
    { "LightGoldenrod2", Colour(238, 220, 130) },
    { "LightGoldenrod3", Colour(205, 190, 112) },
    { "LightGoldenrod4", Colour(139, 129, 76) },
    { "LightGoldenrodYellow", Colour(250, 250, 210) },
    { "LightGray", Colour(211, 211, 211) },
    { "LightGreen", Colour(144, 238, 144) },
    { "LightGrey", Colour(211, 211, 211) },
    { "LightPink", Colour(255, 182, 193) },
    { "LightPink1", Colour(255, 174, 185) },
    { "LightPink2", Colour(238, 162, 173) },
    { "LightPink3", Colour(205, 140, 149) },
    { "LightPink4", Colour(139, 95, 101) },
    { "LightSalmon", Colour(255, 160, 122) },
    { "LightSalmon1", Colour(255, 160, 122) },
    { "LightSalmon2", Colour(238, 149, 114) },
    { "LightSalmon3", Colour(205, 129, 98) },
    { "LightSalmon4", Colour(139, 87, 66) },
    { "LightSeaGreen", Colour(32, 178, 170) },
    { "LightSkyBlue", Colour(135, 206, 250) },
    { "LightSkyBlue1", Colour(176, 226, 255) },
    { "LightSkyBlue2", Colour(164, 211, 238) },
    { "LightSkyBlue3", Colour(141, 182, 205) },
    { "LightSkyBlue4", Colour(96, 123, 139) },
    { "LightSlateBlue", Colour(132, 112, 255) },
    { "LightSlateGray", Colour(119, 136, 153) },
    { "LightSlateGrey", Colour(119, 136, 153) },
    { "LightSteelBlue", Colour(176, 196, 222) },
    { "LightSteelBlue1", Colour(202, 225, 255) },
    { "LightSteelBlue2", Colour(188, 210, 238) },
    { "LightSteelBlue3", Colour(162, 181, 205) },
    { "LightSteelBlue4", Colour(110, 123, 139) },
    { "LightYellow", Colour(255, 255, 224) },
    { "LightYellow1", Colour(255, 255, 224) },
    { "LightYellow2", Colour(238, 238, 209) },
    { "LightYellow3", Colour(205, 205, 180) },
    { "LightYellow4", Colour(139, 139, 122) },
    { "lime", Colour(0, 255, 0) },
    { "lime green", Colour(50, 205, 50) },
    { "LimeGreen", Colour(50, 205, 50) },
    { "linen", Colour(250, 240, 230) },
    { "magenta", Colour(255, 0, 255) },
    { "magenta1", Colour(255, 0, 255) },
    { "magenta2", Colour(238, 0, 238) },
    { "magenta3", Colour(205, 0, 205) },
    { "magenta4", Colour(139, 0, 139) },
    { "maroon", Colour(128, 0, 0) },
    { "maroon1", Colour(255, 52, 179) },
    { "maroon2", Colour(238, 48, 167) },
    { "maroon3", Colour(205, 41, 144) },
    { "maroon4", Colour(139, 28, 98) },
    { "medium aquamarine", Colour(102, 205, 170) },
    { "medium blue", Colour(0, 0, 205) },
    { "medium orchid", Colour(186, 85, 211) },
    { "medium purple", Colour(147, 112, 219) },
    { "medium sea green", Colour(60, 179, 113) },
    { "medium slate blue", Colour(123, 104, 238) },
    { "medium spring green", Colour(0, 250, 154) },
    { "medium turquoise", Colour(72, 209, 204) },
    { "medium violet red", Colour(199, 21, 133) },
    { "MediumAquamarine", Colour(102, 205, 170) },
    { "MediumBlue", Colour(0, 0, 205) },
    { "MediumOrchid", Colour(186, 85, 211) },
    { "MediumOrchid1", Colour(224, 102, 255) },
    { "MediumOrchid2", Colour(209, 95, 238) },
    { "MediumOrchid3", Colour(180, 82, 205) },
    { "MediumOrchid4", Colour(122, 55, 139) },
    { "MediumPurple", Colour(147, 112, 219) },
    { "MediumPurple1", Colour(171, 130, 255) },
    { "MediumPurple2", Colour(159, 121, 238) },
    { "MediumPurple3", Colour(137, 104, 205) },
    { "MediumPurple4", Colour(93, 71, 139) },
    { "MediumSeaGreen", Colour(60, 179, 113) },
    { "MediumSlateBlue", Colour(123, 104, 238) },
    { "MediumSpringGreen", Colour(0, 250, 154) },
    { "MediumTurquoise", Colour(72, 209, 204) },
    { "MediumVioletRed", Colour(199, 21, 133) },
    { "midnight blue", Colour(25, 25, 112) },
    { "MidnightBlue", Colour(25, 25, 112) },
    { "mint cream", Colour(245, 255, 250) },
    { "MintCream", Colour(245, 255, 250) },
    { "misty rose", Colour(255, 228, 225) },
    { "MistyRose", Colour(255, 228, 225) },
    { "MistyRose1", Colour(255, 228, 225) },
    { "MistyRose2", Colour(238, 213, 210) },
    { "MistyRose3", Colour(205, 183, 181) },
    { "MistyRose4", Colour(139, 125, 123) },
    { "moccasin", Colour(255, 228, 181) },
    { "navajo white", Colour(255, 222, 173) },
    { "NavajoWhite", Colour(255, 222, 173) },
    { "NavajoWhite1", Colour(255, 222, 173) },
    { "NavajoWhite2", Colour(238, 207, 161) },
    { "NavajoWhite3", Colour(205, 179, 139) },
    { "NavajoWhite4", Colour(139, 121, 94) },
    { "navy", Colour(0, 0, 128) },
    { "navy blue", Colour(0, 0, 128) },
    { "NavyBlue", Colour(0, 0, 128) },
    { "old lace", Colour(253, 245, 230) },
    { "OldLace", Colour(253, 245, 230) },
    { "olive", Colour(128, 128, 0) },
    { "olive drab", Colour(107, 142, 35) },
    { "OliveDrab", Colour(107, 142, 35) },
    { "OliveDrab1", Colour(192, 255, 62) },
    { "OliveDrab2", Colour(179, 238, 58) },
    { "OliveDrab3", Colour(154, 205, 50) },
    { "OliveDrab4", Colour(105, 139, 34) },
    { "orange", Colour(255, 165, 0) },
    { "orange red", Colour(255, 69, 0) },
    { "orange1", Colour(255, 165, 0) },
    { "orange2", Colour(238, 154, 0) },
    { "orange3", Colour(205, 133, 0) },
    { "orange4", Colour(139, 90, 0) },
    { "OrangeRed", Colour(255, 69, 0) },
    { "OrangeRed1", Colour(255, 69, 0) },
    { "OrangeRed2", Colour(238, 64, 0) },
    { "OrangeRed3", Colour(205, 55, 0) },
    { "OrangeRed4", Colour(139, 37, 0) },
    { "orchid", Colour(218, 112, 214) },
    { "orchid1", Colour(255, 131, 250) },
    { "orchid2", Colour(238, 122, 233) },
    { "orchid3", Colour(205, 105, 201) },
    { "orchid4", Colour(139, 71, 137) },
    { "pale goldenrod", Colour(238, 232, 170) },
    { "pale green", Colour(152, 251, 152) },
    { "pale turquoise", Colour(175, 238, 238) },
    { "pale violet red", Colour(219, 112, 147) },
    { "PaleGoldenrod", Colour(238, 232, 170) },
    { "PaleGreen", Colour(152, 251, 152) },
    { "PaleGreen1", Colour(154, 255, 154) },
    { "PaleGreen2", Colour(144, 238, 144) },
    { "PaleGreen3", Colour(124, 205, 124) },
    { "PaleGreen4", Colour(84, 139, 84) },
    { "PaleTurquoise", Colour(175, 238, 238) },
    { "PaleTurquoise1", Colour(187, 255, 255) },
    { "PaleTurquoise2", Colour(174, 238, 238) },
    { "PaleTurquoise3", Colour(150, 205, 205) },
    { "PaleTurquoise4", Colour(102, 139, 139) },
    { "PaleVioletRed", Colour(219, 112, 147) },
    { "PaleVioletRed1", Colour(255, 130, 171) },
    { "PaleVioletRed2", Colour(238, 121, 159) },
    { "PaleVioletRed3", Colour(205, 104, 127) },
    { "PaleVioletRed4", Colour(139, 71, 93) },
    { "papaya whip", Colour(255, 239, 213) },
    { "PapayaWhip", Colour(255, 239, 213) },
    { "peach puff", Colour(255, 218, 185) },
    { "PeachPuff", Colour(255, 218, 185) },
    { "PeachPuff1", Colour(255, 218, 185) },
    { "PeachPuff2", Colour(238, 203, 173) },
    { "PeachPuff3", Colour(205, 175, 149) },
    { "PeachPuff4", Colour(139, 119, 101) },
    { "peru", Colour(205, 133, 63) },
    { "pink", Colour(255, 192, 203) },
    { "pink1", Colour(255, 181, 197) },
    { "pink2", Colour(238, 169, 184) },
    { "pink3", Colour(205, 145, 158) },
    { "pink4", Colour(139, 99, 108) },
    { "plum", Colour(221, 160, 221) },
    { "plum1", Colour(255, 187, 255) },
    { "plum2", Colour(238, 174, 238) },
    { "plum3", Colour(205, 150, 205) },
    { "plum4", Colour(139, 102, 139) },
    { "powder blue", Colour(176, 224, 230) },
    { "PowderBlue", Colour(176, 224, 230) },
    { "purple", Colour(128, 0, 128) },
    { "purple1", Colour(155, 48, 255) },
    { "purple2", Colour(145, 44, 238) },
    { "purple3", Colour(125, 38, 205) },
    { "purple4", Colour(85, 26, 139) },
    { "red", Colour(255, 0, 0) },
    { "red1", Colour(255, 0, 0) },
    { "red2", Colour(238, 0, 0) },
    { "red3", Colour(205, 0, 0) },
    { "red4", Colour(139, 0, 0) },
    { "rosy brown", Colour(188, 143, 143) },
    { "RosyBrown", Colour(188, 143, 143) },
    { "RosyBrown1", Colour(255, 193, 193) },
    { "RosyBrown2", Colour(238, 180, 180) },
    { "RosyBrown3", Colour(205, 155, 155) },
    { "RosyBrown4", Colour(139, 105, 105) },
    { "royal blue", Colour(65, 105, 225) },
    { "RoyalBlue", Colour(65, 105, 225) },
    { "RoyalBlue1", Colour(72, 118, 255) },
    { "RoyalBlue2", Colour(67, 110, 238) },
    { "RoyalBlue3", Colour(58, 95, 205) },
    { "RoyalBlue4", Colour(39, 64, 139) },
    { "saddle brown", Colour(139, 69, 19) },
    { "SaddleBrown", Colour(139, 69, 19) },
    { "salmon", Colour(250, 128, 114) },
    { "salmon1", Colour(255, 140, 105) },
    { "salmon2", Colour(238, 130, 98) },
    { "salmon3", Colour(205, 112, 84) },
    { "salmon4", Colour(139, 76, 57) },
    { "sandy brown", Colour(244, 164, 96) },
    { "SandyBrown", Colour(244, 164, 96) },
    { "sea green", Colour(46, 139, 87) },
    { "SeaGreen", Colour(46, 139, 87) },
    { "SeaGreen1", Colour(84, 255, 159) },
    { "SeaGreen2", Colour(78, 238, 148) },
    { "SeaGreen3", Colour(67, 205, 128) },
    { "SeaGreen4", Colour(46, 139, 87) },
    { "seashell", Colour(255, 245, 238) },
    { "seashell1", Colour(255, 245, 238) },
    { "seashell2", Colour(238, 229, 222) },
    { "seashell3", Colour(205, 197, 191) },
    { "seashell4", Colour(139, 134, 130) },
    { "sienna", Colour(160, 82, 45) },
    { "sienna1", Colour(255, 130, 71) },
    { "sienna2", Colour(238, 121, 66) },
    { "sienna3", Colour(205, 104, 57) },
    { "sienna4", Colour(139, 71, 38) },
    { "silver", Colour(192, 192, 192) },
    { "sky blue", Colour(135, 206, 235) },
    { "SkyBlue", Colour(135, 206, 235) },
    { "SkyBlue1", Colour(135, 206, 255) },
    { "SkyBlue2", Colour(126, 192, 238) },
    { "SkyBlue3", Colour(108, 166, 205) },
    { "SkyBlue4", Colour(74, 112, 139) },
    { "slate blue", Colour(106, 90, 205) },
    { "slate gray", Colour(112, 128, 144) },
    { "slate grey", Colour(112, 128, 144) },
    { "SlateBlue", Colour(106, 90, 205) },
    { "SlateBlue1", Colour(131, 111, 255) },
    { "SlateBlue2", Colour(122, 103, 238) },
    { "SlateBlue3", Colour(105, 89, 205) },
    { "SlateBlue4", Colour(71, 60, 139) },
    { "SlateGray", Colour(112, 128, 144) },
    { "SlateGray1", Colour(198, 226, 255) },
    { "SlateGray2", Colour(185, 211, 238) },
    { "SlateGray3", Colour(159, 182, 205) },
    { "SlateGray4", Colour(108, 123, 139) },
    { "SlateGrey", Colour(112, 128, 144) },
    { "snow", Colour(255, 250, 250) },
    { "snow1", Colour(255, 250, 250) },
    { "snow2", Colour(238, 233, 233) },
    { "snow3", Colour(205, 201, 201) },
    { "snow4", Colour(139, 137, 137) },
    { "spring green", Colour(0, 255, 127) },
    { "SpringGreen", Colour(0, 255, 127) },
    { "SpringGreen1", Colour(0, 255, 127) },
    { "SpringGreen2", Colour(0, 238, 118) },
    { "SpringGreen3", Colour(0, 205, 102) },
    { "SpringGreen4", Colour(0, 139, 69) },
    { "steel blue", Colour(70, 130, 180) },
    { "SteelBlue", Colour(70, 130, 180) },
    { "SteelBlue1", Colour(99, 184, 255) },
    { "SteelBlue2", Colour(92, 172, 238) },
    { "SteelBlue3", Colour(79, 148, 205) },
    { "SteelBlue4", Colour(54, 100, 139) },
    { "tan", Colour(210, 180, 140) },
    { "tan1", Colour(255, 165, 79) },
    { "tan2", Colour(238, 154, 73) },
    { "tan3", Colour(205, 133, 63) },
    { "tan4", Colour(139, 90, 43) },
    { "teal", Colour(0, 128, 128) },
    { "thistle", Colour(216, 191, 216) },
    { "thistle1", Colour(255, 225, 255) },
    { "thistle2", Colour(238, 210, 238) },
    { "thistle3", Colour(205, 181, 205) },
    { "thistle4", Colour(139, 123, 139) },
    { "tomato", Colour(255, 99, 71) },
    { "tomato1", Colour(255, 99, 71) },
    { "tomato2", Colour(238, 92, 66) },
    { "tomato3", Colour(205, 79, 57) },
    { "tomato4", Colour(139, 54, 38) },
    { "turquoise", Colour(64, 224, 208) },
    { "turquoise1", Colour(0, 245, 255) },
    { "turquoise2", Colour(0, 229, 238) },
    { "turquoise3", Colour(0, 197, 205) },
    { "turquoise4", Colour(0, 134, 139) },
    { "violet", Colour(238, 130, 238) },
    { "violet red", Colour(208, 32, 144) },
    { "VioletRed", Colour(208, 32, 144) },
    { "VioletRed1", Colour(255, 62, 150) },
    { "VioletRed2", Colour(238, 58, 140) },
    { "VioletRed3", Colour(205, 50, 120) },
    { "VioletRed4", Colour(139, 34, 82) },
    { "wheat", Colour(245, 222, 179) },
    { "wheat1", Colour(255, 231, 186) },
    { "wheat2", Colour(238, 216, 174) },
    { "wheat3", Colour(205, 186, 150) },
    { "wheat4", Colour(139, 126, 102) },
    { "white", Colour(255, 255, 255) },
    { "white smoke", Colour(245, 245, 245) },
    { "WhiteSmoke", Colour(245, 245, 245) },
    { "yellow", Colour(255, 255, 0) },
    { "yellow green", Colour(154, 205, 50) },
    { "yellow1", Colour(255, 255, 0) },
    { "yellow2", Colour(238, 238, 0) },
    { "yellow3", Colour(205, 205, 0) },
    { "yellow4", Colour(139, 139, 0) },
    { "YellowGreen", Colour(154, 205, 50) }
};
