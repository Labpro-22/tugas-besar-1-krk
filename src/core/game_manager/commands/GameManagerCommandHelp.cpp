#include "core/game_manager/GameManager.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include "../GameManagerInternal.hpp"
#include "models/base/Property.hpp"
#include "models/tiles/Street.hpp"

namespace {

void appendHelpLine(std::ostringstream& oss, const std::string& command, const std::string& description) {
    oss << command;
    if (command.size() < 24) {
        oss << std::string(static_cast<std::size_t>(24 - command.size()), ' ');
    } else {
        oss << ' ';
    }
    oss << ": " << description << '\n';
}

void appendStateLine(std::ostringstream& oss, const std::string& command, const std::string& description) {
    oss << "- " << command << " : " << description << '\n';
}

bool hasUsableSkillCard(const Player& player) {
    return player.getInventory().getSkillCardCount() > 0;
}

bool hasMortgageableProperty(const Player& player) {
    const std::vector<Property*>& owned = player.getOwnedProperties();
    for (std::size_t i = 0; i < owned.size(); ++i) {
        if (owned[i] != 0 && !owned[i]->isMortgaged()) {
            return true;
        }
    }
    return false;
}

bool hasRedeemableProperty(const Player& player) {
    const std::vector<Property*>& owned = player.getOwnedProperties();
    for (std::size_t i = 0; i < owned.size(); ++i) {
        if (owned[i] != 0 && owned[i]->isMortgaged()) {
            return true;
        }
    }
    return false;
}

bool hasBuildableStreet(const GameManager& game, const Player& player) {
    const std::vector<Property*>& owned = player.getOwnedProperties();
    for (std::size_t i = 0; i < owned.size(); ++i) {
        Street* street = dynamic_cast<Street*>(owned[i]);
        if (street == 0 || street->getOwner() != &player || !street->canBuild()) {
            continue;
        }

        const std::vector<Property*> group = game.getBoard().getPropertiesByColorGroup(street->getColorGroup());
        int minLevel = street->getBuildingLevel();
        for (std::size_t j = 0; j < group.size(); ++j) {
            Street* groupStreet = dynamic_cast<Street*>(group[j]);
            if (groupStreet != 0 && groupStreet->getOwner() == &player) {
                minLevel = std::min(minLevel, groupStreet->getBuildingLevel());
            }
        }

        if (street->getBuildingLevel() == minLevel) {
            return true;
        }
    }
    return false;
}

}

void GameManager::commandHelp() const {
    if (display == 0) return;

    const Player& player = *players[static_cast<std::size_t>(currentPlayerIndex)];
    const bool canRoll = turnPhase == TurnPhase::PRE_ROLL;
    const bool canNext = turnPhase == TurnPhase::POST_RESOLUTION;
    const bool canPayJailFine = player.isInJail() && !hasRolledThisTurn;
    const bool canUseSkill = !hasUsedSkillThisTurn && !hasRolledThisTurn && hasUsableSkillCard(player);
    const bool canSave = !hasRolledThisTurn && !hasUsedSkillThisTurn;
    const bool canMortgage = hasMortgageableProperty(player);
    const bool canUnmortgage = hasRedeemableProperty(player);
    const bool canBuild = hasBuildableStreet(*this, player);

    std::ostringstream oss;
    oss << "=== DAFTAR PERINTAH ===\n";
    appendHelpLine(oss, "HELP", "Tampilkan bantuan perintah.");
    appendHelpLine(oss, "CETAK_PAPAN", "Tampilkan papan permainan saat ini.");
    appendHelpLine(oss, "CETAK_AKTA <kode>", "Tampilkan akta kepemilikan properti.");
    appendHelpLine(oss, "CETAK_PROPERTI", "Tampilkan properti dan kartu kemampuan milik pemain aktif.");
    appendHelpLine(oss, "LEMPAR_DADU", "Lempar dadu untuk bergerak.");
    appendHelpLine(oss, "ATUR_DADU <x> <y>", "Lempar dadu manual dengan nilai 1-6.");
    appendHelpLine(oss, "NEXT", "Lanjutkan atau akhiri giliran setelah resolusi aksi.");
    appendHelpLine(oss, "BAYAR_DENDA", "Bayar denda penjara sebelum melempar dadu.");
    appendHelpLine(oss, "GADAI <kode>", "Gadaikan properti milikmu.");
    appendHelpLine(oss, "TEBUS <kode>", "Tebus kembali properti yang tergadai.");
    appendHelpLine(oss, "BANGUN <kode>", "Bangun rumah atau upgrade hotel.");
    appendHelpLine(oss, "GUNAKAN_KEMAMPUAN <idx>", "Pakai satu kartu kemampuan.");
    appendHelpLine(oss, "SIMPAN <path>", "Simpan state permainan.");
    appendHelpLine(oss, "CETAK_LOG [n]", "Tampilkan log transaksi, opsional n entri terakhir.");
    oss << "\nCatatan: aksi beli, lelang, buang kartu saat tangan penuh, dan pengumuman pemenang berjalan otomatis sesuai kondisi permainan.\n";

    oss << "\n=== Saat Ini Kamu Bisa ===\n";
    appendStateLine(oss, "HELP / CETAK_PAPAN / CETAK_AKTA / CETAK_PROPERTI / CETAK_LOG",
                    "perintah informasi selalu bisa dipakai selama giliranmu.");
    if (canRoll) {
        appendStateLine(oss, "LEMPAR_DADU / ATUR_DADU", "bergerak dengan dadu pada giliran ini.");
    }
    if (canNext) {
        appendStateLine(oss, "NEXT",
                        currentRollWasDouble
                            ? "pilih lempar dadu tambahan atau akhiri giliran."
                            : "akhiri giliran sekarang.");
    }
    if (canPayJailFine) {
        appendStateLine(oss, "BAYAR_DENDA", "keluar dari penjara sebelum melempar dadu.");
    }
    if (canUseSkill) {
        appendStateLine(oss, "GUNAKAN_KEMAMPUAN", "pakai satu kartu kemampuan sebelum melempar dadu.");
    }
    if (canMortgage) {
        appendStateLine(oss, "GADAI", "gadaikan properti yang masih aktif.");
    }
    if (canUnmortgage) {
        appendStateLine(oss, "TEBUS", "tebus properti yang sedang tergadai.");
    }
    if (canBuild) {
        appendStateLine(oss, "BANGUN", "bangun pada street yang memenuhi syarat.");
    }
    if (canSave) {
        appendStateLine(oss, "SIMPAN", "save masih diizinkan karena kamu belum roll dan belum pakai skill.");
    }

    oss << "\n=== Saat Ini Belum Bisa ===\n";
    if (!canRoll) {
        if (turnPhase == TurnPhase::JAIL_CHOICE) {
            appendStateLine(oss, "LEMPAR_DADU / ATUR_DADU",
                            "harus BAYAR_DENDA dulu sebelum boleh melempar dadu.");
        } else {
            appendStateLine(oss, "LEMPAR_DADU / ATUR_DADU",
                            "belum bisa sekarang; selesaikan resolusi aksi atau tunggu giliran baru.");
        }
    }
    if (!canNext) {
        appendStateLine(oss, "NEXT", "hanya bisa dipakai setelah hasil aksi/lempar dadu sudah selesai.");
    }
    if (!canPayJailFine) {
        appendStateLine(oss, "BAYAR_DENDA",
                        player.isInJail()
                            ? "tidak bisa setelah melempar dadu pada giliran ini."
                            : "pemain aktif sedang tidak di penjara.");
    }
    if (!canUseSkill) {
        if (!hasUsableSkillCard(player)) {
            appendStateLine(oss, "GUNAKAN_KEMAMPUAN", "tidak ada kartu kemampuan di tangan.");
        } else if (hasUsedSkillThisTurn) {
            appendStateLine(oss, "GUNAKAN_KEMAMPUAN", "kamu sudah memakai satu skill card pada giliran ini.");
        } else {
            appendStateLine(oss, "GUNAKAN_KEMAMPUAN", "skill card hanya bisa dipakai sebelum melempar dadu.");
        }
    }
    if (!canMortgage) {
        appendStateLine(oss, "GADAI", "tidak ada properti aktif yang bisa digadaikan.");
    }
    if (!canUnmortgage) {
        appendStateLine(oss, "TEBUS", "tidak ada properti tergadai milikmu.");
    }
    if (!canBuild) {
        appendStateLine(oss, "BANGUN", "belum ada street yang memenuhi syarat bangun.");
    }
    if (!canSave) {
        appendStateLine(oss, "SIMPAN", "sudah tidak bisa karena kamu sudah roll atau sudah memakai skill.");
    }

    display->printMessage(oss.str());
}
