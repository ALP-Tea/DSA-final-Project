#include<algorithm>
class GamePlayer final : public Feis::IGamePlayer
{
public:
    PlayerAction GetNextAction(const IGameInfo &info) override 
    {
        if(not_initialize)
        {
            std::string divisor_string = info.GetLevelInfo();
            divisor = std::stoi(divisor_string.substr(1,divisor_string.size()-2));//得到mod用的數字
            Establish_map(info);//掃圖建立簡易
            ActionDetermined(info);//未完成，後續行動決定，預計在裡面呼叫EnqueueAction
            not_initialize = false ;//後面直接返回
        }
        if (actions_.empty()) {
            return {PlayerActionType::None, {0, 0}};
        }
        const PlayerAction action = actions_.front();
        actions_.pop();
        return action;
    }
    void EnqueueAction(const PlayerAction action) { actions_.push(action); }
private:
    void Establish_map(const IGameInfo& info) 
    {
        int height = GameManagerConfig::kBoardHeight;
        int width = GameManagerConfig::kBoardWidth;
        myboard.resize(height, std::vector<int>(width, -9)); // 全部先設為-9
        int centerTop = GameManager::CollectionCenterConfig::kTop;
        int centerLeft = GameManager::CollectionCenterConfig::kLeft;
        int centerSize = GameManagerConfig::kGoalSize;

        for (int r = 0; r < height; ++r) 
        {
            for (int c = 0; c < width; ++c) 
            {
                // 工廠範圍標記
                if (r >= centerTop && r < centerTop + centerSize &&
                    c >= centerLeft && c < centerLeft + centerSize) 
                {
                    myboard[r][c] = -2;
                    continue;
                }
                CellPosition pos{r, c};
                const auto& cell = info.GetLayeredCell(pos); 
                // 判斷牆壁
                auto foreground_ptr = cell.GetForeground().get();
                if (foreground_ptr != nullptr &&
                    dynamic_cast<const WallCell*>(foreground_ptr) != nullptr) {
                    myboard[r][c] = -1;
                    continue;
                }
                // 數字礦石判斷
                auto background_ptr = cell.GetBackground().get();
                if (background_ptr != nullptr) 
                {
                    if (auto num = dynamic_cast<const NumberCell*>(background_ptr)) 
                    {
                        int value = num->GetNumber()%divisor;
                        myboard[r][c] = -5 ;//數字標-5，表示礦
                        if(!value)
                        {
                            usable_mine.push_back({r,c});
                        }
                        else
                        {
                            bad_mine[value] = {r,c};
                        }
                    }
                }
            }
        }
    }
void BuildMiningPath(const IGameInfo& info, CellPosition src, CellPosition dst) {
    std::map<std::pair<int, int>, std::pair<int, int>> parent;
    std::queue<CellPosition> q;
    std::set<std::pair<int, int>> visited;

    q.push(src);
    visited.insert({src.row, src.col});

    std::vector<std::pair<int, int>> dir{{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    std::map<std::pair<int, int>, Direction> dirMap{
        {{-1, 0}, Direction::kTop},
        {{1, 0}, Direction::kBottom},
        {{0, -1}, Direction::kLeft},
        {{0, 1}, Direction::kRight}
    };

    bool found = false;

    while (!q.empty()) {
        CellPosition cur = q.front();
        q.pop();

        if (cur == dst) {
            found = true;
            break;
        }

        for (auto [dr, dc] : dir) {
            int nr = cur.row + dr;
            int nc = cur.col + dc;
            if (nr < 0 || nc < 0 || nr >= (int)myboard.size() || nc >= (int)myboard[0].size())
                continue;
            if (myboard[nr][nc] == -1 || visited.count({nr, nc}))
                continue;

            parent[{nr, nc}] = {cur.row, cur.col};
            visited.insert({nr, nc});
            q.push({nr, nc});
        }
    }

    if (!found) return;

    // 回推路徑
    std::vector<CellPosition> path;
    for (std::pair<int, int> at = {dst.row, dst.col}; at != std::pair<int, int>{src.row, src.col}; at = parent[at]) {
        path.push_back({at.first, at.second});
    }
    std::reverse(path.begin(), path.end());

    // 放礦機
    CellPosition outPos = path.front();
    int dr = outPos.row - src.row;
    int dc = outPos.col - src.col;
    Direction miningDir = dirMap[{dr, dc}];

    switch (miningDir) {
        case Direction::kLeft: EnqueueAction({PlayerActionType::BuildLeftOutMiningMachine, src}); break;
        case Direction::kTop: EnqueueAction({PlayerActionType::BuildTopOutMiningMachine, src}); break;
        case Direction::kRight: EnqueueAction({PlayerActionType::BuildRightOutMiningMachine, src}); break;
        case Direction::kBottom: EnqueueAction({PlayerActionType::BuildBottomOutMiningMachine, src}); break;
    }

    // 放傳送帶
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        CellPosition from = path[i], to = path[i + 1];
        int drow = to.row - from.row;
        int dcol = to.col - from.col;
        Direction dir = dirMap[{drow, dcol}];
        PlayerActionType act;

        switch (dir) {
            case Direction::kLeft: act = PlayerActionType::BuildRightToLeftConveyor; break;
            case Direction::kRight: act = PlayerActionType::BuildLeftToRightConveyor; break;
            case Direction::kTop: act = PlayerActionType::BuildBottomToTopConveyor; break;
            case Direction::kBottom: act = PlayerActionType::BuildTopToBottomConveyor; break;
        }

        EnqueueAction({act, from});
    }
}

    void ActionDetermined(const IGameInfo& info)
    {
        for (auto& mine : usable_mine) {
            BuildMiningPath(info, mine, factoryCenter); // 3. 為能整除的礦建構傳送帶
        }
        handle_bad_mine(info, bad_mine); // 4. 將不能整除的礦合成
    }
    void handle_bad_mine(const IGameInfo& info , std::map<int , CellPosition>& bad_mine)
    {
        
    }
    bool not_initialize = true ;
    std::queue<PlayerAction> actions_;
    bool initial = true;
    std::vector<std::vector<int>>
            myboard;
    int divisor ;
    CellPosition factoryCenter{
        GameManager::CollectionCenterConfig::kTop + GameManagerConfig::kGoalSize / 2,
        GameManager::CollectionCenterConfig::kLeft + GameManagerConfig::kGoalSize / 2
    };
    std::vector<CellPosition> usable_mine ;
    std::map<int , CellPosition> bad_mine ; 
};
