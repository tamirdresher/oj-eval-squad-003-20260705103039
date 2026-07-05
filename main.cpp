#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

enum Status {
    ACCEPTED,
    WRONG_ANSWER,
    RUNTIME_ERROR,
    TIME_LIMIT_EXCEED
};

struct Submission {
    string problem;
    string team;
    Status status;
    int time;
};

struct ProblemStatus {
    int wrongAttempts;      // Wrong attempts before first AC
    int solveTime;          // Time of first AC (-1 if not solved)
    int frozenSubmissions;  // Number of submissions after freeze
    bool isFrozen;          // Whether this problem is frozen
    
    ProblemStatus() : wrongAttempts(0), solveTime(-1), frozenSubmissions(0), isFrozen(false) {}
};

struct Team {
    string name;
    map<string, ProblemStatus> problems;
    int solvedCount;
    int penaltyTime;
    
    Team() : solvedCount(0), penaltyTime(0) {}
    Team(const string& n) : name(n), solvedCount(0), penaltyTime(0) {}
    
    void calculateStats() {
        solvedCount = 0;
        penaltyTime = 0;
        for (auto& p : problems) {
            if (p.second.solveTime != -1 && !p.second.isFrozen) {
                solvedCount++;
                penaltyTime += 20 * p.second.wrongAttempts + p.second.solveTime;
            }
        }
    }
    
    vector<int> getSolveTimes() const {
        vector<int> times;
        for (const auto& p : problems) {
            if (p.second.solveTime != -1 && !p.second.isFrozen) {
                times.push_back(p.second.solveTime);
            }
        }
        sort(times.rbegin(), times.rend());
        return times;
    }
};

class ICPCSystem {
private:
    map<string, Team> teams;
    vector<string> teamOrder;
    bool started;
    int durationTime;
    int problemCount;
    vector<string> problemIds;
    bool frozen;
    vector<Submission> submissions;
    
    bool compareTeams(const string& t1, const string& t2) {
        Team& team1 = teams[t1];
        Team& team2 = teams[t2];
        
        // More solved problems
        if (team1.solvedCount != team2.solvedCount) {
            return team1.solvedCount > team2.solvedCount;
        }
        
        // Less penalty time
        if (team1.penaltyTime != team2.penaltyTime) {
            return team1.penaltyTime < team2.penaltyTime;
        }
        
        // Compare solve times
        vector<int> times1 = team1.getSolveTimes();
        vector<int> times2 = team2.getSolveTimes();
        
        int minSize = min(times1.size(), times2.size());
        for (int i = 0; i < minSize; i++) {
            if (times1[i] != times2[i]) {
                return times1[i] < times2[i];
            }
        }
        
        // Lexicographic order
        return t1 < t2;
    }
    
    void updateRankings() {
        for (auto& t : teams) {
            t.second.calculateStats();
        }
        
        sort(teamOrder.begin(), teamOrder.end(), [this](const string& a, const string& b) {
            return compareTeams(a, b);
        });
    }
    
    void printScoreboard() {
        for (int i = 0; i < teamOrder.size(); i++) {
            Team& team = teams[teamOrder[i]];
            cout << team.name << " " << (i + 1) << " " << team.solvedCount << " " << team.penaltyTime;
            
            for (const string& pid : problemIds) {
                cout << " ";
                if (team.problems.find(pid) == team.problems.end()) {
                    cout << ".";
                } else {
                    ProblemStatus& ps = team.problems[pid];
                    if (ps.isFrozen) {
                        if (ps.wrongAttempts == 0) {
                            cout << "0/" << ps.frozenSubmissions;
                        } else {
                            cout << "-" << ps.wrongAttempts << "/" << ps.frozenSubmissions;
                        }
                    } else if (ps.solveTime != -1) {
                        if (ps.wrongAttempts == 0) {
                            cout << "+";
                        } else {
                            cout << "+" << ps.wrongAttempts;
                        }
                    } else {
                        if (ps.wrongAttempts == 0) {
                            cout << ".";
                        } else {
                            cout << "-" << ps.wrongAttempts;
                        }
                    }
                }
            }
            cout << "\n";
        }
    }
    
    Status parseStatus(const string& s) {
        if (s == "Accepted") return ACCEPTED;
        if (s == "Wrong_Answer") return WRONG_ANSWER;
        if (s == "Runtime_Error") return RUNTIME_ERROR;
        return TIME_LIMIT_EXCEED;
    }
    
public:
    ICPCSystem() : started(false), frozen(false), durationTime(0), problemCount(0) {}
    
    void addTeam(const string& teamName) {
        if (started) {
            cout << "[Error]Add failed: competition has started.\n";
            return;
        }
        if (teams.find(teamName) != teams.end()) {
            cout << "[Error]Add failed: duplicated team name.\n";
            return;
        }
        teams[teamName] = Team(teamName);
        teamOrder.push_back(teamName);
        cout << "[Info]Add successfully.\n";
    }
    
    void startCompetition(int duration, int problems) {
        if (started) {
            cout << "[Error]Start failed: competition has started.\n";
            return;
        }
        started = true;
        durationTime = duration;
        problemCount = problems;
        
        for (int i = 0; i < problems; i++) {
            problemIds.push_back(string(1, 'A' + i));
        }
        
        cout << "[Info]Competition starts.\n";
    }
    
    void submit(const string& problem, const string& team, const string& statusStr, int time) {
        Status status = parseStatus(statusStr);
        submissions.push_back({problem, team, status, time});
        
        Team& t = teams[team];
        ProblemStatus& ps = t.problems[problem];
        
        if (frozen) {
            if (ps.solveTime == -1) {
                ps.frozenSubmissions++;
                if (ps.frozenSubmissions == 1) {
                    ps.isFrozen = true;
                }
            }
        } else {
            if (ps.solveTime == -1) {
                if (status == ACCEPTED) {
                    ps.solveTime = time;
                } else {
                    ps.wrongAttempts++;
                }
            }
        }
    }
    
    void flush() {
        updateRankings();
        cout << "[Info]Flush scoreboard.\n";
    }
    
    void freeze() {
        if (frozen) {
            cout << "[Error]Freeze failed: scoreboard has been frozen.\n";
            return;
        }
        frozen = true;
        cout << "[Info]Freeze scoreboard.\n";
    }
    
    void scroll() {
        if (!frozen) {
            cout << "[Error]Scroll failed: scoreboard has not been frozen.\n";
            return;
        }
        
        cout << "[Info]Scroll scoreboard.\n";
        
        updateRankings();
        printScoreboard();
        
        while (true) {
            bool hasFreeze = false;
            for (auto& t : teams) {
                for (auto& p : t.second.problems) {
                    if (p.second.isFrozen) {
                        hasFreeze = true;
                        break;
                    }
                }
                if (hasFreeze) break;
            }
            
            if (!hasFreeze) break;
            
            // Find lowest ranked team with frozen problems
            string targetTeam = "";
            string targetProblem = "";
            for (int i = teamOrder.size() - 1; i >= 0; i--) {
                Team& team = teams[teamOrder[i]];
                string minProblem = "";
                for (auto& p : team.problems) {
                    if (p.second.isFrozen) {
                        if (minProblem.empty() || p.first < minProblem) {
                            minProblem = p.first;
                        }
                    }
                }
                if (!minProblem.empty()) {
                    targetTeam = team.name;
                    targetProblem = minProblem;
                    break;
                }
            }
            
            // Unfreeze the problem
            Team& team = teams[targetTeam];
            ProblemStatus& ps = team.problems[targetProblem];
            
            int oldRank = 0;
            for (int i = 0; i < teamOrder.size(); i++) {
                if (teamOrder[i] == targetTeam) {
                    oldRank = i;
                    break;
                }
            }
            
            // Process frozen submissions
            for (const auto& sub : submissions) {
                if (sub.team == targetTeam && sub.problem == targetProblem) {
                    if (frozen && ps.frozenSubmissions > 0) {
                        if (ps.solveTime == -1) {
                            if (sub.status == ACCEPTED) {
                                ps.solveTime = sub.time;
                            } else {
                                ps.wrongAttempts++;
                            }
                        }
                        ps.frozenSubmissions--;
                        if (ps.frozenSubmissions == 0) {
                            ps.isFrozen = false;
                            break;
                        }
                    }
                }
            }
            
            ps.isFrozen = false;
            updateRankings();
            
            int newRank = 0;
            for (int i = 0; i < teamOrder.size(); i++) {
                if (teamOrder[i] == targetTeam) {
                    newRank = i;
                    break;
                }
            }
            
            if (newRank < oldRank) {
                cout << targetTeam << " " << teamOrder[newRank + 1] << " " << team.solvedCount << " " << team.penaltyTime << "\n";
            }
        }
        
        printScoreboard();
        frozen = false;
    }
    
    void queryRanking(const string& teamName) {
        if (teams.find(teamName) == teams.end()) {
            cout << "[Error]Query ranking failed: cannot find the team.\n";
            return;
        }
        
        cout << "[Info]Complete query ranking.\n";
        if (frozen) {
            cout << "[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\n";
        }
        
        int rank = 0;
        for (int i = 0; i < teamOrder.size(); i++) {
            if (teamOrder[i] == teamName) {
                rank = i + 1;
                break;
            }
        }
        
        cout << teamName << " NOW AT RANKING " << rank << "\n";
    }
    
    void querySubmission(const string& teamName, const string& problem, const string& status) {
        if (teams.find(teamName) == teams.end()) {
            cout << "[Error]Query submission failed: cannot find the team.\n";
            return;
        }
        
        cout << "[Info]Complete query submission.\n";
        
        Submission* result = nullptr;
        for (int i = submissions.size() - 1; i >= 0; i--) {
            if (submissions[i].team != teamName) continue;
            if (problem != "ALL" && submissions[i].problem != problem) continue;
            if (status != "ALL") {
                string subStatus;
                if (submissions[i].status == ACCEPTED) subStatus = "Accepted";
                else if (submissions[i].status == WRONG_ANSWER) subStatus = "Wrong_Answer";
                else if (submissions[i].status == RUNTIME_ERROR) subStatus = "Runtime_Error";
                else subStatus = "Time_Limit_Exceed";
                
                if (subStatus != status) continue;
            }
            result = &submissions[i];
            break;
        }
        
        if (result == nullptr) {
            cout << "Cannot find any submission.\n";
        } else {
            string statusStr;
            if (result->status == ACCEPTED) statusStr = "Accepted";
            else if (result->status == WRONG_ANSWER) statusStr = "Wrong_Answer";
            else if (result->status == RUNTIME_ERROR) statusStr = "Runtime_Error";
            else statusStr = "Time_Limit_Exceed";
            
            cout << result->team << " " << result->problem << " " << statusStr << " " << result->time << "\n";
        }
    }
    
    void end() {
        cout << "[Info]Competition ends.\n";
    }
};

int main() {
    ICPCSystem system;
    string line;
    
    while (getline(cin, line)) {
        istringstream iss(line);
        string cmd;
        iss >> cmd;
        
        if (cmd == "ADDTEAM") {
            string teamName;
            iss >> teamName;
            system.addTeam(teamName);
        } else if (cmd == "START") {
            string duration, prob;
            int durationTime, problemCount;
            iss >> duration >> durationTime >> prob >> problemCount;
            system.startCompetition(durationTime, problemCount);
        } else if (cmd == "SUBMIT") {
            string problem, by, team, with, status, at;
            int time;
            iss >> problem >> by >> team >> with >> status >> at >> time;
            system.submit(problem, team, status, time);
        } else if (cmd == "FLUSH") {
            system.flush();
        } else if (cmd == "FREEZE") {
            system.freeze();
        } else if (cmd == "SCROLL") {
            system.scroll();
        } else if (cmd == "QUERY_RANKING") {
            string teamName;
            iss >> teamName;
            system.queryRanking(teamName);
        } else if (cmd == "QUERY_SUBMISSION") {
            string teamName, where, condition;
            getline(iss, teamName, ' ');
            getline(iss, teamName, ' ');
            
            string rest;
            getline(iss, rest);
            
            size_t problemPos = rest.find("PROBLEM=");
            size_t statusPos = rest.find("STATUS=");
            
            string problem = rest.substr(problemPos + 8, rest.find(" AND", problemPos) - problemPos - 8);
            string status = rest.substr(statusPos + 7);
            
            system.querySubmission(teamName, problem, status);
        } else if (cmd == "END") {
            system.end();
            break;
        }
    }
    
    return 0;
}
