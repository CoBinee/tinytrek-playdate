// Game.c - ゲーム
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Scene.h"
#include "Actor.h"
#include "Aseprite.h"
#include "Application.h"
#include "Game.h"
#include "Console.h"
#include "Display.h"
#include "Report.h"

// 内部関数
//
static void GameUnload(struct Game *game);
static void GameTransition(struct Game *game, GameFunction function);
static void GameLoad(struct Game *game);
static void GamePlay(struct Game *game);
static void GameDone(struct Game *game);
static void GameBasicNext(struct Game *game);
static void GameBasicBack(struct Game *game);
static void GameBasicGoto(struct Game *game, int line);
static void GameBasicGosub(struct Game *game, int line);
static void GameBasicReturn(struct Game *game);

// 内部変数
//
static const char *gameSpriteNames[] = {
    "", 
};
static const char *gameAudioSamplePaths[] = {
    "", 
};
static const char *gameAudioMusicPath = "";


// ゲームを更新する
//
void GameUpdate(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game == NULL) {

        // ゲームの作成
        game = playdate->system->realloc(NULL, sizeof (struct Game));
        if (game == NULL) {
            playdate->system->error("%s: %d: game instance is not created.", __FILE__, __LINE__);
            return;
        }
        memset(game, 0, sizeof (struct Game));

        // ゲームの初期化
        {
            // ユーザデータの設定
            SceneSetUserdata(game);

            // 解放の設定
            SceneSetUnload((SceneFunction)GameUnload);

            // クラシックの設定
            game->classic = true;
        }

        // スプライトの読み込み
        // AsepriteLoadSpriteList(gameSpriteNames, kGameSpriteNameSize);

        // オーディオの読み込み
        // IocsLoadAudioEffects(gameAudioSamplePaths, kGameAudioSampleSize);

        // コンソールの初期化
        ConsoleInitialize();

        // ディスプレイの初期化
        DisplayInitialize();

        // レポートの初期化
        ReportInitialize();

        // 処理の設定
        GameTransition(game, (GameFunction)GameLoad);
    }

    // 処理の更新
    if (game->function != NULL) {
        (*game->function)(game);
    }
}

// ゲームを解放する
//
static void GameUnload(struct Game *game)
{
    // アクタの解放
    ActorUnloadAll();

    // スプライトの解放
    AsepriteUnloadAllSprites();

    // オーディオの解放
    IocsUnloadAllAudioEffects();
}

// 処理を遷移する
//
static void GameTransition(struct Game *game, GameFunction function)
{
    game->function = function;
    game->state = 0;
}

// ゲームを読み込むする
//
static void GameLoad(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        //コンソールの読み込み
        ConsoleLoad();

        // 初期化の完了
        ++game->state;
    }

    // 処理の遷移
    GameTransition(game, (GameFunction)GamePlay);
}

// ゲームをプレイする
//
static void GamePlay(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // Basic の設定
        GameBasicGoto(game, 0);

        // 初期化の完了
        ++game->state;
    }

    // テキストを表示している
    if (ConsoleIsPrintText()) {
        ;

    // メニューが開かれている
    } else if (ConsoleIsOpenMenu()) {
        ;
    
    // 数値が入力されている
    } else if (ConsoleIsInputNumber()) {
        ;
    
    // 角度が入力されている
    } else if (ConsoleIsInputAngle()) {
        ;
    
    // BASIC の実行
    } else {

        // 0 
        if (game->basic.run.line == 0) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("DO YOU PLAY A CLASSIC GAME? ");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                static const char *items[] = {
                    "NO", 
                    "YES", 
                };
                ConsoleOpenMenu(items, 2);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 2) {
                if (ConsoleGetSelectedMenu() == 0) {
                    DisplayLoad();
                    ReportLoad();
                    game->classic = false;
                } else {
                    game->classic = true;
                }
                GameBasicGoto(game, 5);
            }

        // 5 Y=2999;IN."DO YOU WANT A DIFFICULT GAME?  ","(Y OR N)"A
        } else if (game->basic.run.line == 5) {
            if (game->basic.run.sentence == 0) {
                game->basic.Y = 2999;
                ConsolePrintText("DO YOU WANT A DIFFICULT GAME? ");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                static const char *items[] = {
                    "NO", 
                    "YES", 
                };
                ConsoleOpenMenu(items, 2);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 2) {
                game->basic.A = ConsoleGetSelectedMenu();
                GameBasicGoto(game, 10);
            }

        // 10 PR."STARDATE 3200:  YOUR MISSION IS ",;IFA=YY=999
        // 15 K=0,B=0,D=30;F.I=0TO63;J=R.(99)<5,B=B+J
        // 20 M=R.(Y),M=(M<209)+(M<99)+(M<49)+(M<24)+(M<9)+(M<2),K=K+M
        // 25 @(I)=-100*M-10*J-R.(8);N.I;IF(B<2)+(K<4)G.15
        // 30 PR."TO DESTROY ",#1,K," KLINGONS IN 30 STARDATES.
        // 35 PR."THERE ARE ",#1,B," STARBASES.";GOS.160;C=0,H=K
        } else if (game->basic.run.line == 10) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.A == 1) {
                    game->basic.Y = 999;
                }
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                game->basic.K = 0;
                game->basic.B = 0;
                game->basic.D = 30;
                for (game->basic.I = 0; game->basic.I <= 63; game->basic.I++) {
                    game->basic.J = (rand() % 99 + 1) < 5 ? 1 : 0;
                    game->basic.B = game->basic.B + game->basic.J;
                    game->basic.M = (rand() % game->basic.Y + 1);
                    game->basic.M 
                        = (game->basic.M < 209 ? 1 : 0) 
                        + (game->basic.M < 99 ? 1 : 0) 
                        + (game->basic.M < 49 ? 1 : 0) 
                        + (game->basic.M < 24 ? 1 : 0) 
                        + (game->basic.M < 9 ? 1 : 0)
                        + (game->basic.M < 2 ? 1 : 0);
                    game->basic.K = game->basic.K + game->basic.M;
                    game->basic.at[game->basic.I] = -100 * game->basic.M - 10 * game->basic.J - (rand() % 8 + 1);
                }
                if (game->basic.B < 2 || game->basic.K < 4) {
                    ;
                } else {
                    char *text;
                    playdate->system->formatString(&text, "STARDATE 3200: YOUR MISSION IS TO DESTROY %d KLINGONS IN 30 STARDATES. THERE ARE %d STARBASES.\n", game->basic.K, game->basic.B);
                    strcpy(game->text, text);
                    playdate->system->realloc(text, 0);
                    ConsolePrintText(game->text);
                    GameBasicGosub(game, 160);
                }
            } else if (game->basic.run.sentence == 2) {
                game->basic.C = 0;
                game->basic.H = game->basic.K;
                GameBasicGoto(game, 40);
            }
        
        // 40 U=R.(8),V=R.(8),X=R.(8),Y=R.(8)
        } else if (game->basic.run.line == 40) {
            if (game->basic.run.sentence == 0) {
                game->basic.U = rand() % 8 + 1;
                game->basic.V = rand() % 8 + 1;
                game->basic.X = rand() % 8 + 1;
                game->basic.Y = rand() % 8 + 1;
                GameBasicGoto(game, 45);
            }

        // 45 F.I=71TO152;@(I)=0;N.I;@(8*X+Y+62)=4,M=A.(@(8*U+V-9)),N=M/100
        } else if (game->basic.run.line == 45) {
            if (game->basic.run.sentence == 0) {
                for (game->basic.I = 71; game->basic.I <= 152; game->basic.I++) {
                    game->basic.at[game->basic.I] = 0;
                }
                game->basic.at[8 * game->basic.X + game->basic.Y + 62] = 4;
                game->basic.M = abs(game->basic.at[8 * game->basic.U + game->basic.V - 9]);
                game->basic.N = game->basic.M / 100;
                GameBasicGoto(game, 50);
            }

        // 50 I=1;IFNF.J=1TON;GOS.165;@(J+134)=300,@(J+140)=S,@(J+146)=T;N.J
        } else if (game->basic.run.line == 50) {
            if (game->basic.run.sentence == 0) {
                game->basic.I = 1;
                if (game->basic.N != 0) {
                    game->basic.J = 1;
                    GameBasicNext(game);
                } else {
                    GameBasicGoto(game, 55);
                }
            } else if (game->basic.run.sentence == 1) {
                GameBasicGosub(game, 165);
            } else if (game->basic.run.sentence == 2) {
                game->basic.at[game->basic.J + 134] = 300;
                game->basic.at[game->basic.J + 140] = game->basic.S;
                game->basic.at[game->basic.J + 146] = game->basic.T;
                ++game->basic.J;
                if (game->basic.J <= game->basic.N) {
                    GameBasicBack(game);
                } else {
                    GameBasicGoto(game, 55);
                }
            }

        // 55 GOS.175;M=M-100*N,I=2;IFM/10GOS.165
        } else if (game->basic.run.line == 55) {
            if (game->basic.run.sentence == 0) {
                GameBasicGosub(game, 175);
            } else if (game->basic.run.sentence == 1) {
                game->basic.M = game->basic.M - 100 * game->basic.N;
                game->basic.I = 2;
                if (game->basic.M / 10 != 0) {
                    GameBasicGosub(game, 165);
                } else {
                    GameBasicGoto(game, 60);
                }
            } else if (game->basic.run.sentence == 2) {
                GameBasicGoto(game, 60);
            }

        // 60 M=M-M/10*10,I=3;IFMF.J=1TOM;GOS.165;N.J
        } else if (game->basic.run.line == 60) {
            if (game->basic.run.sentence == 0) {
                game->basic.M = game->basic.M - game->basic.M / 10 * 10;
                game->basic.I = 3;
                if (game->basic.M != 0) {
                    game->basic.J = 1;
                    GameBasicNext(game);
                } else {
                    GameBasicGoto(game, 65);
                }
            } else if (game->basic.run.sentence == 1) {
                GameBasicGosub(game, 165);
            } else if (game->basic.run.sentence == 2) {
                ++game->basic.J;
                if (game->basic.J <= game->basic.M) {
                    GameBasicBack(game);
                } else {
                    GameBasicGoto(game, 65);
                }
            }

        // 65 GOS.145;GOS.325;IFKG.95
        } else if (game->basic.run.line == 65) {
            if (game->basic.run.sentence == 0) {
                GameBasicGosub(game, 145);
            } else if (game->basic.run.sentence == 1) {
                GameBasicGosub(game, 325);
            } else if (game->basic.run.sentence == 2) {
                if (game->basic.K != 0) {
                    GameBasicGoto(game, 95);
                } else {
                    GameBasicGoto(game, 70);
                }
            }

        // 70 PR.;PR."MISSION ACCOMPLISHED.";IFD<3PR."BOY, YOU BARELY MADE IT. 
        } else if (game->basic.run.line == 70) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("\nMISSION ACCOMPLISHED.\n");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.D < 3) {
                    ConsolePrintText("BOY, YOU BARELY MADE IT.\n");
                }
                GameBasicGoto(game, 75);
            }

        // 75 IFD>5PR."GOOD WORK...";IFD>9PR."FANTASTIC!";IFD>13PR."UNBELIEVABLE!
        } else if (game->basic.run.line == 75) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.D > 5) {
                    ConsolePrintText("GOOD WORK...\n");
                    GameBasicNext(game);
                } else {
                    GameBasicGoto(game, 80);
                }
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.D > 9) {
                    ConsolePrintText("FANTASTIC!\n");
                    GameBasicNext(game);
                } else {
                    GameBasicGoto(game, 80);
                }
            } else if (game->basic.run.sentence ==2) {
                if (game->basic.D > 13) {
                    ConsolePrintText("UNBELIEVABLE!\n");
                }
                GameBasicGoto(game, 80);
            }

        // 80 D=30-D,I=H*100/D*10;PR.#1,H," KLINGONS IN ",D," STARDATES. (",I,")
        } else if (game->basic.run.line == 80) {
            if (game->basic.run.sentence == 0) {
                game->basic.D = 30 - game->basic.D;
                game->basic.I = game->basic.H * 100 / game->basic.D * 10;
                char *text;
                playdate->system->formatString(&text, "%d KLINGONS IN %d STARDATES. (%d)\n", game->basic.H, game->basic.D, game->basic.I);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicGoto(game, 85);
            }

        // 85 J=100*(C=0)-5*C;PR.#1,C," CASUALTIES INCURRED. (",J,")
        } else if (game->basic.run.line == 85) {
            if (game->basic.run.sentence == 0) {
                game->basic.J = 100 * (game->basic.C == 0 ? 1 : 0) - 5 * game->basic.C;
                char *text;
                playdate->system->formatString(&text, "%d CASUALTIES INCURRED. (%d)\n", game->basic.C, game->basic.J);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicGoto(game, 90);
            }

        // 90 PR."YOUR SCORE:",I+J;G.110
        } else if (game->basic.run.line == 90) {
            if (game->basic.run.sentence == 0) {
                char *text;
                playdate->system->formatString(&text, "YOUR SCORE: %d\n", game->basic.I + game->basic.J);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicGoto(game, 110);
            }

        // 95 IFD<0PR."IT'S TOO LATE, THE FEDERATION HAS BEEN CONQUERED.";G.110
        } else if (game->basic.run.line == 95) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.D < 0) {
                    ConsolePrintText("IT'S TOO LATE, THE FEDERATION HAS BEEN CONQUERED.n");
                    GameBasicGoto(game, 110);
                } else {
                    GameBasicGoto(game, 100);
                }
            }

        // 100 IFE>=0G.120
        } else if (game->basic.run.line == 100) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.E >= 0) {
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 105);
                }
            }

        // 105 PR."ENTERPRISE DESTROYED";IFH-K>9PR."BUT YOU WERE A GOOD MAN
        } else if (game->basic.run.line == 105) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("ENTERPRISE DESTROYED\n");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.H - game->basic.K > 9) {
                    ConsolePrintText("BUT YOU WERE A GOOD MAN\n");
                }
                GameBasicGoto(game, 110);
            }

        // 110 Y=987;PR.;IN."ANOTHERE GAME?  ","(Y OR N)"A;IFA=YG.5
        } else if (game->basic.run.line == 110) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("\nANOTHERE GAME? ");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                static const char *items[] = {
                    "NO", 
                    "YES", 
                };
                ConsoleOpenMenu(items, 2);
                GameBasicNext(game);
            } else {
                game->basic.A = ConsoleGetSelectedMenu();
                if (game->basic.A == 1) {
                    GameBasicGoto(game, 5);
                } else {
                    GameBasicGoto(game, 115);
                }
            }

        // 115 PR."GOOD BYE.";STOP
        } else if (game->basic.run.line == 115) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("GOOD BYE.");
                GameBasicNext(game);
            } else {
                ;
            }
        
        // 120 S=220,G=180,L=200,P=260,R=420,W=465,T=555;IN."CAPTAIN"A
        // 125 IF(A=S)+(A=G)+(A=L)+(A=P)+(A=R)+(A=W)+(A=T)G.A
        // 130 PR."R=REPORT       S=SR. SENSOR   L=LR. SENSOR
        // 135 PR."G=GALAXY MAP   P=PHASER       T=TORPEDO
        // 140 PR."W=WARP ENGINE  **PLEASE USE ONE OF THESE COMMANDS***";G.120
        } else if (game->basic.run.line == 120) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("CAPTAIN? ");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                static const char *items[] = {
                    "REPORT", 
                    "SHORT RANGE SENSOR", 
                    "LONG RANGE SENSOR", 
                    "GALAXY MAP", 
                    "PHASER", 
                    "TORPEDO", 
                    "WARP ENGINE"
                };
                ConsoleOpenMenu(items, 7);
                GameBasicNext(game);
            } else {
                game->basic.A = ConsoleGetSelectedMenu();
                // 120 S=220,G=180,L=200,P=260,R=420,W=465,T=555;IN."CAPTAIN"A
                if (game->basic.A == 0) {
                    GameBasicGoto(game, 420);
                } else if (game->basic.A == 1) {
                    GameBasicGoto(game, 220);
                } else if (game->basic.A == 2) {
                    GameBasicGoto(game, 200);
                } else if (game->basic.A == 3) {
                    GameBasicGoto(game, 180);
                } else if (game->basic.A == 4) {
                    GameBasicGoto(game, 260);
                } else if (game->basic.A == 5) {
                    GameBasicGoto(game, 555);
                } else if (game->basic.A == 6) {
                    GameBasicGoto(game, 465);
                }
            }
        
        // 145 F.I=X-(X>1)TOX+(X<8);F.J=Y-(Y>1)TOY+(Y<8)
        // 150 IF@(8*I+J+62)#2N.J;N.I;O=0;R.
        } else if (game->basic.run.line == 145) {
            if (game->basic.run.sentence == 0) {
                game->basic.I = game->basic.X - (game->basic.X > 1 ? 1 : 0);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                game->basic.J = game->basic.Y - (game->basic.Y > 1 ? 1 : 0);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 2) {
                if (game->basic.at[8 * game->basic.I + game->basic.J + 62] != 2) {
                    ++game->basic.J;
                    if (game->basic.J <= game->basic.Y + (game->basic.Y < 8 ? 1 : 0)) {
                        ;
                    } else {
                        ++game->basic.I;
                        if (game->basic.I <= game->basic.X + (game->basic.X < 8 ? 1 : 0)) {
                            GameBasicBack(game);
                        } else {
                            game->basic.O = 0;
                            GameBasicReturn(game);
                        }
                    }
                } else {
                    GameBasicGoto(game, 155);
                }
            }
        
        // 155 IFO=0PR.'SULU: "CAPTAIN, WE ARE DOCKED AT STARBASE."
        } else if (game->basic.run.line == 155) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.O == 0) {
                    ConsolePrintText("SULU: \"CAPTAIN, WE ARE DOCKED AT STARBASE.\"\n");
                }
                GameBasicGoto(game, 160);
            }

        // 160 E=4000,F=10,O=1;F.I=64TO70;@(I)=0;N.I;R.
        } else if (game->basic.run.line == 160) {
            if (game->basic.run.sentence == 0) {
                game->basic.E = 4000;
                game->basic.F = 10;
                game->basic.O = 1;
                for (game->basic.I = 64; game->basic.I <= 70; game->basic.I++) {
                    game->basic.at[game->basic.I] = 0;
                }
                GameBasicReturn(game);
            }

        // 165 S=R.(8),T=R.(8),A=8*S+T+62;IF@(A)G.165
        // 170 @(A)=I;R.
        } else if (game->basic.run.line == 165) {
            if (game->basic.run.sentence == 0) {
                game->basic.S = rand() % 8 + 1;
                game->basic.T = rand() % 8 + 1;
                game->basic.A = 8 * game->basic.S + game->basic.T + 62;
                if (game->basic.at[game->basic.A] != 0) {
                    ;
                } else {
                    GameBasicNext(game);
                }
            } else if (game->basic.run.sentence == 1) {
                game->basic.at[game->basic.A] = game->basic.I;
                GameBasicReturn(game);
            }

        // 175 PR."ENTERPRISE IN Q-",#1,U,V," S-",X,Y;R.
        } else if (game->basic.run.line == 175) {
            if (game->basic.run.sentence == 0) {
                char *text;
                playdate->system->formatString(&text, "ENTERPRISE IN Q-%d%d S-%d%d\n", game->basic.U, game->basic.V, game->basic.X, game->basic.Y);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                GameBasicReturn(game);
            }

        // 180 GOS.175;J=2;GOS.375;IFIG.120
        } else if (game->basic.run.line == 180) {
            if (game->basic.run.sentence == 0) {
                GameBasicGosub(game, 175);
            } else if (game->basic.run.sentence == 1) {
                game->basic.J = 2;
                GameBasicGosub(game, 375);
            } else if (game->basic.run.sentence == 2) {
                if (!game->classic) {
                    DisplaySetMap(kDisplayMapGalaxy);
                }
                if (game->basic.I != 0) {
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 185);
                }
            }

        // 185 PR." OF GALAXY MAP";F.I=0TO7;PR.;PR.#1,I+1,":",;F.J=0TO7;M=@(8*I+J)
        // 190 PR.#4,(M>0)*M,;N.J;PR.;N.I;PR."  ",;F.I=0TO7;PR."  ..",;N.I;PR.
        // 195 PR."  ",;F.I=1TO8;PR.#4,I,;N.I;PR.;PR.;G.120
        } else if (game->basic.run.line == 185) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText(" OF GALAXY MAP\n\n");
                game->text[0] = '\0';
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                for (game->basic.I = 0; game->basic.I <= 7; game->basic.I++) {
                    {
                        char *text;
                        playdate->system->formatString(&text, "%d:", game->basic.I + 1);
                        strcat(game->text, text);
                        playdate->system->realloc(text, 0);
                    }
                    for (game->basic.J = 0; game->basic.J <= 7; game->basic.J++) {
                        game->basic.M = game->basic.at[8 * game->basic.I + game->basic.J];
                        {
                            char *text;
                            playdate->system->formatString(&text, " %03d", (game->basic.M > 0 ? 1 : 0) * game->basic.M);
                            strcat(game->text, text);
                            playdate->system->realloc(text, 0);
                        }
                    }
                    strcat(game->text, "\n");
                }
                strcat(game->text, "  ");
                for (game->basic.I = 0; game->basic.I <= 7; game->basic.I++) {
                    strcat(game->text, "  ..");
                }
                strcat(game->text, "\n");
                strcat(game->text, "  ");
                for (game->basic.I = 1; game->basic.I <= 8; game->basic.I++) {
                    char *text;
                    playdate->system->formatString(&text, "% 4d", game->basic.I);
                    strcat(game->text, text);
                    playdate->system->realloc(text, 0);
                }
                strcat(game->text, "\n\n");
                if (game->classic) {
                    ConsolePrintText(game->text);
                }
                GameBasicGoto(game, 120);
            }

        // 200 GOS.175;J=3;GOS.375;IFIG.120
        } else if (game->basic.run.line == 200) {
            if (game->basic.run.sentence == 0) {
                GameBasicGosub(game, 175);
            } else if (game->basic.run.sentence == 1) {
                game->basic.J = 3;
                GameBasicGosub(game, 375);
            } else if (game->basic.run.sentence == 2) {
                if (!game->classic) {
                    DisplaySetMap(kDisplayMapGalaxy);
                }
                if (game->basic.I != 0) {
                    GameBasicGoto(game, 120);
                } else {
                    ConsolePrintText("\n");
                    GameBasicGoto(game, 205);
                }
            }

        // 205 PR.;F.I=U-1TOU+1;F.J=V-1TOV+1;M=8*I+J-9,A=0
        // 210 IF(I>0)*(I<9)*(J>0)*(J<9)A=A.(@(M)),@(M)=A
        // 215 PR.#4,A,;N.J;PR.;N.I;G.120
        } else if (game->basic.run.line == 205) {
            if (game->basic.run.sentence == 0) {
                strcpy(game->text, "\n");
                for (game->basic.I = game->basic.U - 1; game->basic.I <= game->basic.U + 1; game->basic.I++) {
                    for (game->basic.J = game->basic.V - 1; game->basic.J <= game->basic.V + 1; game->basic.J++) {
                        game->basic.M = 8 * game->basic.I + game->basic.J - 9;
                        game->basic.A = 0;
                        if (game->basic.I > 0 && game->basic.I < 9 && game->basic.J > 0 && game->basic.J < 9) {
                            game->basic.A = abs(game->basic.at[game->basic.M]);
                            game->basic.at[game->basic.M] = game->basic.A;
                        }
                        {
                            char *text;
                            playdate->system->formatString(&text, "% 4d", game->basic.A);
                            strcat(game->text, text);
                            playdate->system->realloc(text, 0);
                        }
                    }
                    strcat(game->text, "\n");
                }
                strcat(game->text, "\n");
                if (game->classic) {
                    ConsolePrintText(game->text);
                } else {
                    ConsolePrintText("\n");
                }
                GameBasicGoto(game, 120);
            }

        // 220 GOS.175;J=1;GOS.375;IFIG.120
        } else if (game->basic.run.line == 220) {
            if (game->basic.run.sentence == 0) {
                GameBasicGosub(game, 175);
            } else if (game->basic.run.sentence == 1) {
                game->basic.J = 1;
                GameBasicGosub(game, 375);
            } else if (game->basic.run.sentence == 2) {
                if (!game->classic) {
                    DisplaySetMap(kDisplayMapSector);
                }
                if (game->basic.I != 0) {
                    GameBasicGoto(game, 120);
                } else {
                    ConsolePrintText("\n");
                    GameBasicGoto(game, 225);
                }
            }

        // 225 M=8*U+V-9,@(M)=A.(@(M))
        // 230 PR.;F.I=1TO8;PR.#1,I,;F.J=1TO8;M=@(8*I+J+62);IFM=0PR." .",
        // 235 IFM=1PR." K",
        // 240 IFM=2PR." B",
        // 245 IFM=3PR." *",
        // 250 IFM=4PR." E",
        // 255 N.J;PR.;N.I;PR." ",;F.I=1TO8;PR.#2,I,;N.I;PR.;G.120
        } else if (game->basic.run.line == 225) {
            if (game->basic.run.sentence == 0) {
                game->basic.M = 8 * game->basic.U + game->basic.V - 9;
                game->basic.at[game->basic.M] = abs(game->basic.at[game->basic.M]);
                strcpy(game->text, "\n");
                for (game->basic.I = 1; game->basic.I <= 8; game->basic.I++) {
                    char *text;
                    playdate->system->formatString(&text, "%d", game->basic.I);
                    strcat(game->text, text);
                    playdate->system->realloc(text, 0);
                    for (game->basic.J = 1; game->basic.J <= 8; game->basic.J++) {
                        game->basic.M = game->basic.at[8 * game->basic.I + game->basic.J + 62];
                        if (game->basic.M == 0) {
                            strcat(game->text, " .");
                        } else if (game->basic.M == 1) {
                            strcat(game->text, " K");
                        } else if (game->basic.M == 2) {
                            strcat(game->text, " B");
                        } else if (game->basic.M == 3) {
                            strcat(game->text, " *");
                        } else if (game->basic.M == 4) {
                            strcat(game->text, " E");
                        }
                    }
                    strcat(game->text, "\n");
                }
                strcat(game->text, " ");
                for (game->basic.I = 1; game->basic.I <= 8; game->basic.I++) {
                    char *text;
                    playdate->system->formatString(&text, "% 2d", game->basic.I);
                    strcat(game->text, text);
                    playdate->system->realloc(text, 0);
                }
                strcat(game->text, "\n\n");
                if (game->classic) {
                    ConsolePrintText(game->text);
                } else {
                    ConsolePrintText("\n");
                }
                GameBasicGoto(game, 120);
            }

        // 260 J=4;GOS.375;IFIG.120
        } else if (game->basic.run.line == 260) {
            if (game->basic.run.sentence == 0) {
                game->basic.J = 4;
                GameBasicGosub(game, 375);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.I != 0) {
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 265);
                }
            }

        // 265 IN." ENERGIZED. ","UNITS TO FIRE"A;IFA<1G.120
        } else if (game->basic.run.line == 265) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText(" ENERGIZED. UNITS TO FIRE? ");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                ConsoleInputNumber(1, 1, game->basic.E);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 2) {
                game->basic.A = ConsoleGetInputedNumber();
                if (game->basic.A < 1) {
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 270);
                }
            }

        // 270 IFA>EPR.'SPOCK: "WE HAVE ONLY ',#1,E,' UNITS."';G.120
        } else if (game->basic.run.line == 270) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.A > game->basic.E) {
                    char *text;
                    playdate->system->formatString(&text, "SPOCK: \"WE HAVE ONLY %d UNITS.\"\n", game->basic.E);
                    strcpy(game->text, text);
                    playdate->system->realloc(text, 0);
                    ConsolePrintText(game->text);
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 275);
                }
            }

        // 275 E=E-A;IFN<1PR."PHASER FIRED AT EMPTY SPACE.";G.65
        } else if (game->basic.run.line == 275) {
            if (game->basic.run.sentence == 0) {
                game->basic.E = game->basic.E - game->basic.A;
                if (game->basic.N < 1) {
                    ConsolePrintText("PHASER FIRED AT EMPTY SPACE.\n\n");
                    GameBasicGoto(game, 65);
                } else {
                    GameBasicGoto(game, 280);
                }
            }

        // 280 A=A/N;F.M=135TO140;IF@(M)=0G.290
        // 285 GOS.295;PR.#3,S," UNITS HIT ",;GOS.305
        // 290 N.M;G.65
        } else if (game->basic.run.line == 280) {
            if (game->basic.run.sentence == 0) {
                game->basic.A = game->basic.A / game->basic.N;
                game->basic.M = 135;
                GameBasicGoto(game, 285);
            }
        } else if (game->basic.run.line == 285) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.at[game->basic.M] == 0) {
                    GameBasicGoto(game, 290);
                } else {
                    GameBasicGosub(game, 295);
                }
            } if (game->basic.run.sentence == 1) {
                char *text;
                playdate->system->formatString(&text, "%d UNITS HIT ", game->basic.S);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicGosub(game, 305);
            } if (game->basic.run.sentence == 2) {
                GameBasicGoto(game, 290);
            }
        } else if (game->basic.run.line == 290) {
            if (game->basic.run.sentence == 0) {
                ++game->basic.M;
                if (game->basic.M <= 140) {
                    GameBasicGoto(game, 285);
                } else {
                    ConsolePrintText("\n");
                    GameBasicGoto(game, 65);
                }
            }

        // 295 IFA>1090PR."...OVERLOADED..";J=4,@(67)=1,A=9;GOS.375
        // 300 I=@(M+6)-X,J=@(M+12)-Y,S=A*30/(30+I*I+J*J)+1;R.
        } else if (game->basic.run.line == 295) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.A > 1090) {
                    ConsolePrintText("...OVERLOADED..\n");
                    game->basic.J = 4;
                    game->basic.at[67] = 1;
                    game->basic.A = 9;
                    GameBasicGosub(game, 375);
                } else {
                    GameBasicNext(game);
                }
            } else if (game->basic.run.sentence == 1) {
                game->basic.I = game->basic.at[game->basic.M + 6] - game->basic.X;
                game->basic.J = game->basic.at[game->basic.M + 12] - game->basic.Y;
                game->basic.S = game->basic.A * 30 / (30 + game->basic.I * game->basic.I + game->basic.J * game->basic.J) + 1;
                GameBasicReturn(game);
            }

        // 305 PR."KLINGON AT S-",#1,@(M+6),@(M+12),;@(M)=@(M)-S
        } else if (game->basic.run.line == 305) {
            if (game->basic.run.sentence == 0) {
                char *text;
                playdate->system->formatString(&text, "KLINGON AT S-%d%d", game->basic.at[game->basic.M + 6], game->basic.at[game->basic.M + 12]);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                game->basic.at[game->basic.M] = game->basic.at[game->basic.M] - game->basic.S;
                GameBasicGoto(game, 310);
            }

        // 310 IF@(M)>0PR." **DAMAGED**";R.
        } else if (game->basic.run.line == 310) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.at[game->basic.M] > 0) {
                    ConsolePrintText(" **DAMAGED**\n");
                    GameBasicReturn(game);
                } else {
                    GameBasicGoto(game, 315);
                }
            }

        // 315 @(M)=0,I=8*U+V-9,J=@(I)/A.(@(I)),@(I)=@(I)-100*J,K=K-1
        } else if (game->basic.run.line == 315) {
            if (game->basic.run.sentence == 0) {
                game->basic.at[game->basic.M] = 0;
                game->basic.I = 8 * game->basic.U + game->basic.V - 9;
                game->basic.J = game->basic.at[game->basic.I] / abs(game->basic.at[game->basic.I]);
                game->basic.at[game->basic.I] = game->basic.at[game->basic.I] - 100 * game->basic.J;
                game->basic.K = game->basic.K - 1;
                GameBasicGoto(game, 320);
            }

        // 320 I=8*@(M+6)+@(M+12)+62,@(I)=0,N=N-1;PR." ***DESTROYED***";R.
        } else if (game->basic.run.line == 320) {
            if (game->basic.run.sentence == 0) {
                game->basic.I = 8 * game->basic.at[game->basic.M + 6] + game->basic.at[game->basic.M + 12] + 62;
                game->basic.at[game->basic.I] = 0;
                game->basic.N = game->basic.N - 1;
                ConsolePrintText(" ***DESTROYED***\n");
                GameBasicReturn(game);
            }

        // 325 IFN=0R.
        } else if (game->basic.run.line == 325) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.N == 0) {
                    GameBasicReturn(game);
                } else {
                    GameBasicGoto(game, 330);
                }
            }

        // 330 PR."KLINGON ATTACK";IFOPR."STARBASE PROTECTS ENTERPRISE";R.
        } else if (game->basic.run.line == 330) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("KLINGON ATTACK\n");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.O != 0) {
                    ConsolePrintText("STARBASE PROTECTS ENTERPRISE\n");
                    GameBasicReturn(game);
                } else {
                    GameBasicGoto(game, 335);
                }
            }

        // 335 T=0;F.M=135TO140;IF@(M)=0G.350
        // 340 A=(@(M)+R.(@(M)))/2;GOS.295;T=T+S,I=@(M+6),J=@(M+12)
        // 345 PR.#3,S," UNITS HIT FROM KLINGON AT S-",#1,I,J
        } else if (game->basic.run.line == 335) {
            if (game->basic.run.sentence == 0) {
                game->basic.T = 0;
                game->basic.M = 135;
                GameBasicGoto(game, 336);
            }
        } else if (game->basic.run.line == 336) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.at[game->basic.M] == 0) {
                    GameBasicGoto(game, 350);
                } else {
                    game->basic.A = (game->basic.at[game->basic.M] + (rand() % game->basic.at[game->basic.M] + 1)) / 2;
                    GameBasicGosub(game, 295);
                }
            } else if (game->basic.run.sentence == 1) {
                game->basic.T = game->basic.T + game->basic.S;
                game->basic.I = game->basic.at[game->basic.M + 6];
                game->basic.J = game->basic.at[game->basic.M + 12];
                char *text;
                playdate->system->formatString(&text, "%d UNITS HIT FROM KLINGON AT S-%d%d\n", game->basic.S, game->basic.I, game->basic.J);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicGoto(game, 350);
            } 

        // 350 N.M;E=E-T;IFE<=0PR."*** BANG ***";R.
        } else if (game->basic.run.line == 350) {
            if (game->basic.run.sentence == 0) {
                ++game->basic.M;
                if (game->basic.M <= 140) {
                    GameBasicGoto(game, 336);
                } else {
                    GameBasicNext(game);
                }
            } else if (game->basic.run.sentence == 1) {
                game->basic.E = game->basic.E - game->basic.T;
                if (game->basic.E <= 0) {
                    ConsolePrintText("*** BANG ***\n");
                    GameBasicReturn(game);
                } else {
                    GameBasicGoto(game, 355);
                }
            }

        // 355 PR.#1,E," UNITS OF ENERGY LEFT.";IFR.(E/4)>TR.
        } else if (game->basic.run.line == 355) {
            if (game->basic.run.sentence == 0) {
                char *text;
                playdate->system->formatString(&text, "%d UNITS OF ENERGY LEFT.\n", game->basic.E);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                if ((rand() % (game->basic.E / 4) + 1) > game->basic.T) {
                    GameBasicReturn(game);
                } else {
                    GameBasicGoto(game, 360);
                }
            }

        // 360 IF@(70)=0@(70)=R.(T/50+1),J=7;G.375
        } else if (game->basic.run.line == 360) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.at[70] == 0) {
                    game->basic.at[70] = rand() % (game->basic.T / 50 + 1) + 1;
                    game->basic.J = 7;
                    GameBasicGoto(game, 375);
                } else {
                    GameBasicGoto(game, 365);
                }
            }

        // 365 J=R.(6),@(J+63)=R.(T/99+1)+@(J+63),I=R.(8)+1,C=C+I
        } else if (game->basic.run.line == 365) {
            if (game->basic.run.sentence == 0) {
                game->basic.J = rand() % 6 + 1;
                game->basic.at[game->basic.J + 63] = rand() % (game->basic.T / 99 + 1) + 1 + game->basic.at[game->basic.J + 63];
                game->basic.I = (rand() % 8 + 1) + 1;
                game->basic.C = game->basic.C + game->basic.I;
                GameBasicGoto(game, 370);
            }

        // 370 PR.'MC COY: "SICKBAY TO BRIDGE, WE SUFFERED',#2,I,' CASUALTIES."
        } else if (game->basic.run.line == 370) {
            if (game->basic.run.sentence == 0) {
                char *text;
                playdate->system->formatString(&text, "MC COY: \"SICKBAY TO BRIDGE, WE SUFFERED %d CASUALTIES.\"\n", game->basic.I);
                strcpy(game->text, text);
                playdate->system->realloc(text, 0);
                ConsolePrintText(game->text);
                GameBasicGoto(game, 375);
            }

        // 375 I=@(J+63);IFJ=1PR."SHORT RANGE SENSOR",
        // 380 IFJ=2PR."COMPUTER DISPLAY",
        // 385 IFJ=3PR."LONG RANGE SENSOR",
        // 390 IFJ=4PR."PHASER",
        // 395 IFJ=5PR."WARP ENGINE",
        // 400 IFJ=6PR."PHOTON TORPEDO TUBES",
        // 405 IFJ=7PR."SHIELD",
        // 410 IFI=0R.
        // 415 PR." DAMAGED, ",#1,I," STARDATES ESTIMATED FOR REPAIR";R.
        } else if (game->basic.run.line == 375) {
            if (game->basic.run.sentence == 0) {
                 game->basic.I = game->basic.at[game->basic.J + 63];
                 if (game->basic.J == 1) {
                     strcpy(game->text, "SHORT RANGE SENSOR");
                 } else if (game->basic.J == 2) {
                     strcpy(game->text, "COMPUTER DISPLAY");
                 } else if (game->basic.J == 3) {
                     strcpy(game->text, "LONG RANGE SENSOR");
                 } else if (game->basic.J == 4) {
                     strcpy(game->text, "PHASER");
                 } else if (game->basic.J == 5) {
                     strcpy(game->text, "WARP ENGINE");
                 } else if (game->basic.J == 6) {
                     strcpy(game->text, "PHOTON TORPEDO TUBES");
                 } else if (game->basic.J == 7) {
                     strcpy(game->text, "SHIELD");
                 }
                 if (game->basic.I == 0) {
                    ConsolePrintText(game->text);
                 } else {
                    char *text;
                    playdate->system->formatString(&text, " DAMAGED, %d STARDATES ESTIMATED FOR REPAIR\n", game->basic.I);
                    strcat(game->text, text);
                    playdate->system->realloc(text, 0);
                    ConsolePrintText(game->text);
                 }
                 GameBasicReturn(game);
            }

        // 420 PR."STATUS REPORT:";PR."STARDATE",#10,3230-D;PR."TIME LEFT",#7,D
        // 425 PR."CONDITION     ",;IFOPR."DOCKED";G.445
        // 430 IFNPR."RED";G.445
        // 435 IFE<999PR."YELLOW";G.445
        // 440 PR."GREEN
        // 445 PR."POSITION      Q-",#1,U,V," S-",X,Y;PR."ENERGY",#12,E
        // 450 PR."TORPEDOES",#7,F;PR."KLINGONS LEFT",#3,K;PR."STARBASES",#6,B
        } else if (game->basic.run.line == 420) {
            if (game->basic.run.sentence == 0) {
                {
                    char *text;
                    playdate->system->formatString(&text, "STATUS REPORT:\n\nSTARDATE      %d\nTIME LEFT     %d\n", 3230 - game->basic.D, game->basic.D);
                    strcpy(game->text, text);
                    playdate->system->realloc(text, 0);
                }
                strcat(game->text, "CONDITION     ");
                if (game->basic.O != 0) {
                    strcat(game->text, "DOCKED\n");
                } else if (game->basic.N != 0) {
                    strcat(game->text, "RED\n");
                } else if (game->basic.E < 999) {
                    strcat(game->text, "YELLOW\n");
                } else {
                    strcat(game->text, "GREEN\n");
                }
                {
                    char *text;
                    playdate->system->formatString(&text, "POSITION      Q-%d%d S-%d%d\nENERGY        %d\n", game->basic.U, game->basic.V, game->basic.X, game->basic.Y, game->basic.E);
                    strcat(game->text, text);
                    playdate->system->realloc(text, 0);
                }
                {
                    char *text;
                    playdate->system->formatString(&text, "TORPEDOES     %d\nKLINGONS LEFT %d\nSTARBASES     %d\n\n", game->basic.F, game->basic.K, game->basic.B);
                    strcat(game->text, text);
                    playdate->system->realloc(text, 0);
                }
                ConsolePrintText(game->text);
                GameBasicGoto(game, 455);
            }

        // 455 F.J=1TO7;IF@(J+63)GOS.375
        // 460 N.J;G.120
        } else if (game->basic.run.line == 455) {
            if (game->basic.run.sentence == 0) {
                game->basic.J = 1;
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.at[game->basic.J + 63] != 0) {
                    GameBasicGosub(game, 375);
                } else {
                    GameBasicNext(game);
                }
            } else if (game->basic.run.sentence == 2) {
                ++game->basic.J;
                if (game->basic.J <= 7) {
                    GameBasicBack(game);
                } else {
                    GameBasicGoto(game, 120);
                }
            }

        // 465 J=5;GOS.375;IFI=0PR.
        } else if (game->basic.run.line == 465) {
            if (game->basic.run.sentence == 0) {
                game->basic.J = 5;
                GameBasicGosub(game, 375);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.I == 0) {
                    ConsolePrintText("\n");
                }
                GameBasicGoto(game, 470);
            }

        // 470 IN."SECTOR DISTANCE"W;IFW<1G.120
        } else if (game->basic.run.line == 470) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("SECTOR DISTANCE? ");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                ConsoleInputNumber(1, 1, 91);
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 2) {
                game->basic.W = ConsoleGetInputedNumber();
                if (game->basic.W < 0) {
                    ConsolePrintText("\n");
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 475);
                }
            }

        // 475 IFI*(W>2)PR.'CHEKOV: "WE CAN TRY 2 AT MOST, SIR."';G.470
        } else if (game->basic.run.line == 475) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.I * (game->basic.W > 2 ? 1 : 0) != 0) {
                    ConsolePrintText("CHEKOV: \"WE CAN TRY 2 AT MOST, SIR.\"\n");
                    GameBasicGoto(game, 470);
                } else {
                    GameBasicGoto(game, 480);
                }
            }

        // 480 IFW>91W=91;PR.'SPOCK: "ARE YOU SURE, CAPTAIN?"
        // 485 IFE<W*W/2PR.'SCOTTY: "SIR, WE DO NOT HAVE THE ENERGY."';G.120
        } else if (game->basic.run.line == 480) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.E < game->basic.W * game->basic.W / 2) {
                    ConsolePrintText("SCOTTY: \"SIR, WE DO NOT HAVE THE ENERGY.\"\n\n");
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 490);
                }
            }

        // 490 GOS.615;IFR=0G.120
        } else if (game->basic.run.line == 490) {
            if (game->basic.run.sentence == 0) {
                GameBasicGosub(game, 615);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.R == 0) {
                    ConsolePrintText("\n");
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 495);
                }
            }

        // 495 D=D-1,E=E-W*W/2,@(8*X+Y+62)=0
        // 500 F.M=64TO70;@(M)=(@(M)-1)*(@(M)>0);N.M
        } else if (game->basic.run.line == 495) {
            if (game->basic.run.sentence == 0) {
                game->basic.D = game->basic.D - 1;
                game->basic.E = game->basic.E - game->basic.W * game->basic.W / 2;
                game->basic.at[8 * game->basic.X + game->basic.Y + 62] = 0;
                for (game->basic.M = 64; game->basic.M <= 70; game->basic.M++) {
                    game->basic.at[game->basic.M] = (game->basic.at[game->basic.M] - 1) * (game->basic.at[game->basic.M] > 0 ? 1 : 0);
                }
                GameBasicGoto(game, 505);
            }

        // 505 P=45*X+22,G=45*Y+22,W=45*W;F.M=1TO8;W=W-R;IFW<-22G.525
        // 510 P=P+S,G=G+T,I=P/45,J=G/45;IF(I<1)+(I>8)+(J<1)+(J>8)G.530
        // 515 IF@(8*I+J+62)=0X=I,Y=J;N.M
        } else if (game->basic.run.line == 505) {
            if (game->basic.run.sentence == 0) {
                game->basic.P = 45 * game->basic.X + 22;
                game->basic.G = 45 * game->basic.Y + 22;
                game->basic.W = 45 * game->basic.W;
                game->basic.M = 1;
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                game->basic.W = game->basic.W - game->basic.R;
                if (game->basic.W < -22) {
                    GameBasicGoto(game, 525);
                } else {
                    game->basic.P = game->basic.P + game->basic.S;
                    game->basic.G = game->basic.G + game->basic.T;
                    game->basic.I = game->basic.P / 45;
                    game->basic.J = game->basic.G / 45;
                    if (game->basic.I < 1 || game->basic.I > 8 || game->basic.J < 1 || game->basic.J >8) {
                        GameBasicGoto(game, 530);
                    } else {
                        if (game->basic.at[8 * game->basic.I + game->basic.J + 62] == 0) {
                            game->basic.X = game->basic.I;
                            game->basic.Y = game->basic.J;
                            ++game->basic.M;
                            if (game->basic.M <= 8) {
                                ;
                            } else {
                                GameBasicGoto(game, 520);
                            }
                        }
                    }
                }
            }

        // 520 PR."**EMERGENCY STOP**";PR.'SPOCK: "TO ERR IS HUMAN."
        } else if (game->basic.run.line == 520) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("**EMERGENCY STOP**\nSPOCK: \"TO ERR IS HUMAN.\"\n");
                GameBasicGoto(game, 525);
            }

        // 525 @(8*X+Y+62)=4;GOS.175;G.65
        } else if (game->basic.run.line == 525) {
            if (game->basic.run.sentence == 0) {
                game->basic.at[8 * game->basic.X + game->basic.Y + 62] = 4;
                GameBasicGosub(game, 175);
            } else if (game->basic.run.sentence == 1) {
                ConsolePrintText("\n");
                GameBasicGoto(game, 65);
            }

        // 530 P=U*72+P/5+W/5*S/R-9,U=P/72,G=V*72+G/5+W/5*T/R-9,V=G/72
        } else if (game->basic.run.line == 530) {
            if (game->basic.run.sentence == 0) {
                game->basic.P = game->basic.U * 72 + game->basic.P / 5 + game->basic.W / 5 * game->basic.S / game->basic.R - 9;
                game->basic.U = game->basic.P / 72;
                game->basic.G = game->basic.V * 72 + game->basic.G / 5 + game->basic.W / 5 * game->basic.T / game->basic.R - 9;
                game->basic.V = game->basic.G / 72;
                GameBasicGoto(game, 535);
            }

        // 535 IFR.(9)<2PR."***SPACE STORM***";T=100;GOS.360
         } else if (game->basic.run.line == 535) {
            if (game->basic.run.sentence == 0) {
                if (rand() % 9 + 1 < 2) {
                    ConsolePrintText("***SPACE STORM***\n");
                    GameBasicNext(game);
                } else {
                    GameBasicGoto(game, 540);
                }
            } else if (game->basic.run.sentence == 1) {
                game->basic.T = 100;
                GameBasicGosub(game, 360);
            } else if (game->basic.run.sentence == 2) {
                GameBasicGoto(game, 540);
            }

        // 540 IF(U>0)*(U<9)*(V>0)*(V<9)X=(P+9-72*U)/9,Y=(G+9-72*V)/9;G.45
         } else if (game->basic.run.line == 540) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.U > 0 && game->basic.U < 9 && game->basic.V > 0 && game->basic.V < 9) {
                    game->basic.X = (game->basic.P + 9 - 72 * game->basic.U) / 9;
                    game->basic.Y = (game->basic.G + 9 - 72 * game->basic.V) / 9;
                    ConsolePrintText("\n");
                    GameBasicGoto(game, 45);
                } else {
                    GameBasicGoto(game, 545);
                }
            }

        // 545 PR."**YOU WANDERED OUTSIDE THE GALAXY**
        // 550 PR."ON BOARD COMPUTER TAKES OVER, AND SAVED YOUR LIFE";G.40
         } else if (game->basic.run.line == 545) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("**YOU WANDERED OUTSIDE THE GALAXY**\nON BOARD COMPUTER TAKES OVER, AND SAVED YOUR LIFE\n\n");
                GameBasicGoto(game, 40);
            }

        // 555 J=6;GOS.375;IFIG.120
         } else if (game->basic.run.line == 555) {
            if (game->basic.run.sentence == 0) {
                game->basic.J = 6;
                GameBasicGosub(game, 375);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.I != 0) {
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 560);
                }
            }

        // 560 IFF=0PR." EMPTY";G.120
        } else if (game->basic.run.line == 560) {
            if (game->basic.run.sentence == 0) {
                if (game->basic.F == 0) {
                    ConsolePrintText(" EMPTY\n");
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 565);
                }
            }

        // 565 PR." LOADED";GOS.615;IFR=0G.120
        } else if (game->basic.run.line == 565) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText(" LOADED\n");
                GameBasicGosub(game, 615);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.R == 0) {
                    GameBasicGoto(game, 120);
                } else {
                    GameBasicGoto(game, 570);
                }
            }

        // 570 PR."TORPEDO TRACK ",;F=F-1,P=45*X+22,G=45*Y+22;F.M=1TO8
        } else if (game->basic.run.line == 570) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("TORPEDO TRACK ");
                game->basic.F = game->basic.F - 1;
                game->basic.P = 45 * game->basic.X + 22;
                game->basic.G = 45 * game->basic.Y + 22;
                game->basic.M = 1;
                GameBasicGoto(game, 575);
            }

        // 575 P=P+S,G=G+T,I=P/45,J=G/45;IF(I<1)+(I>8)+(J<1)+(J>8)G.585
        } else if (game->basic.run.line == 575) {
            if (game->basic.run.sentence == 0) {
                game->basic.P = game->basic.P + game->basic.S;
                game->basic.G = game->basic.G + game->basic.T;
                game->basic.I = game->basic.P / 45;
                game->basic.J = game->basic.G / 45;
                if (game->basic.I < 1 || game->basic.I > 8 || game->basic.J < 1 || game->basic.J > 8) {
                    GameBasicGoto(game, 585);
                } else {
                    GameBasicGoto(game, 580);
                }
            }

        // 580 L=8*I+J+62,W=8*U+V-9,R=@(W)/A.(@(W));PR.#1,I,J," ",;G.585+5*@(L)
        } else if (game->basic.run.line == 580) {
            if (game->basic.run.sentence == 0) {
                game->basic.L = 8 * game->basic.I + game->basic.J + 62;
                game->basic.W = 8 * game->basic.U + game->basic.V - 9;
                game->basic.R = game->basic.at[game->basic.W] / abs(game->basic.at[game->basic.W]);
                {
                    char *text;
                    playdate->system->formatString(&text, "%d%d ", game->basic.I, game->basic.J);
                    strcpy(game->text, text);
                    playdate->system->realloc(text, 0);
                    ConsolePrintText(game->text);
                }
                GameBasicGoto(game, 585 + 5 * game->basic.at[game->basic.L]);
            }

        // 585 N.M;PR.;G.65
        } else if (game->basic.run.line == 585) {
            if (game->basic.run.sentence == 0) {
                ++game->basic.M;
                if (game->basic.M <= 8) {
                    GameBasicGoto(game, 575);
                } else {
                    ConsolePrintText("...MISSED\n\n");
                    GameBasicGoto(game, 65);
                }
            }

        // 590 S=R.(99)+280;F.M=135TO140;IF(@(M+6)=I)*(@(M+12)=J)GOS.305
        // 592 N.M;G.65
        } else if (game->basic.run.line == 590) {
            if (game->basic.run.sentence == 0) {
                game->basic.S = (rand() % 99 + 1) + 280;
                game->basic.M = 135;
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                if (game->basic.at[game->basic.M + 6] == game->basic.I && game->basic.at[game->basic.M + 12] == game->basic.J) {
                    GameBasicGosub(game, 305);
                } else {
                    GameBasicNext(game);
                }
            } else if (game->basic.run.sentence == 2) {
                ++game->basic.M;
                if (game->basic.M <= 140) {
                    GameBasicBack(game);
                } else {
                    ConsolePrintText("\n");
                    GameBasicGoto(game, 65);
                }
            }

        // 595 B=B-1,@(L)=0,@(W)=@(W)-10*R;PR."STARBASE DESTROYED
        // 597 PR.'SPOCK: "I OFTEN FIND HUMAN BEHAVIOUR FASCINATING."';G.65
        } else if (game->basic.run.line == 595) {
            if (game->basic.run.sentence == 0) {
                game->basic.B = game->basic.B - 1;
                game->basic.at[game->basic.L] = 0;
                game->basic.at[game->basic.W] = game->basic.at[game->basic.W] - 10 * game->basic.R;
                ConsolePrintText("STARBASE DESTROYED\n");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                ConsolePrintText("SPOCK: \"I OFTEN FIND HUMAN BEHAVIOUR FASCINATING.\"\n\n");
                GameBasicGoto(game, 65);
            }

        // 600 PR."HIT A STAR";IFR.(9)<3PR."TORPEDO ABSORBED";G.65
        } else if (game->basic.run.line == 600) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("HIT A STAR\n");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                if ((rand() % 9 + 1) < 3) {
                    ConsolePrintText("TORPEDO ABSORBED\n\n");
                    GameBasicGoto(game, 65);
                } else {
                    GameBasicGoto(game, 605);
                }
            }

        // 605 @(L)=0,@(W)=@(W)-R;IFR.(9)<6PR."STAR DESTROYED";G.65
        } else if (game->basic.run.line == 605) {
            if (game->basic.run.sentence == 0) {
                game->basic.at[game->basic.L] = 0;
                game->basic.at[game->basic.W] = game->basic.at[game->basic.W] - game->basic.R;
                if (rand() % 9 + 1 < 6) {
                    ConsolePrintText("STAR DESTROYED\n\n");
                    GameBasicGoto(game, 65);
                } else {
                    GameBasicGoto(game, 610);
                }
            }

        // 610 T=300;PR."IT NOVAS    ***RADIATION ALARM***";GOS.360;G.65
        } else if (game->basic.run.line == 610) {
            if (game->basic.run.sentence == 0) {
                game->basic.T = 300;
                ConsolePrintText("IT NOVAS    ***RADIATION ALARM***\n");
                GameBasicGosub(game, 360);
            } else if (game->basic.run.sentence == 1) {
                ConsolePrintText("\n");
                GameBasicGoto(game, 65);
            }

        // 615 IN."COURSE (0-360)"I;IF(I>360)+(I<0)R=0;R.
        // 620 S=(I+45)/90,I=I-S*90,R=(45+I*I)/110+45;G.625+5*(S<4)*S
        // 625 S=-45,T=I;R.
        // 630 S=I,T=45;R.
        // 635 S=45,T=-I;R.
        // 640 S=-I,T=-45;R.
        } else if (game->basic.run.line == 615) {
            if (game->basic.run.sentence == 0) {
                ConsolePrintText("COURSE (0-360)? ");
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 1) {
                ConsoleInputAngle();
                GameBasicNext(game);
            } else if (game->basic.run.sentence == 2) {
                game->basic.I = ConsoleGetInputedAngle();
                game->basic.S = (game->basic.I + 45) / 90;
                game->basic.I = game->basic.I - game->basic.S * 90;
                game->basic.R = (45 + game->basic.I * game->basic.I) / 110 + 45;
                int i = 625 + 5 * (game->basic.S < 4 ? 1 : 0) * game->basic.S;
                if (i == 625) {
                    game->basic.S = -45;
                    game->basic.T = game->basic.I;
                } else if (i == 630) {
                    game->basic.S = game->basic.I;
                    game->basic.T = 45;
                } else if (i == 635) {
                    game->basic.S = 45;
                    game->basic.T = -game->basic.I;
                } else {
                    game->basic.S = -game->basic.I;
                    game->basic.T = -45;
                }
                GameBasicReturn(game);
            }

        // ERROR
        } else {
            playdate->system->error("%s: %d: illegal basic line: %d.", __FILE__, __LINE__, game->basic.run.line);
        }
    }
}

// ゲームを完了する
//
static void GameDone(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // 初期化の完了
        ++game->state;
    }

    // シーンの遷移
    ApplicationTransition(kApplicationSceneTitle);
}

// BASIC を処理する
//
static void GameBasicNext(struct Game *game)
{
    ++game->basic.run.sentence;
}
static void GameBasicBack(struct Game *game)
{
    --game->basic.run.sentence;
}
static void GameBasicGoto(struct Game *game, int line)
{
    game->basic.run.line = line;
    game->basic.run.sentence = 0;
}
static void GameBasicGosub(struct Game *game, int line)
{
    if (game->basic.returnStack < kGameBasicReturnSize) {
        game->basic.returns[game->basic.returnStack].line = game->basic.run.line;
        game->basic.returns[game->basic.returnStack].sentence = game->basic.run.sentence + 1;
        ++game->basic.returnStack;
        game->basic.run.line = line;
        game->basic.run.sentence = 0;
    }
}
static void GameBasicReturn(struct Game *game)
{
    if (game->basic.returnStack > 0) {
        --game->basic.returnStack;
        game->basic.run.line = game->basic.returns[game->basic.returnStack].line;
        game->basic.run.sentence = game->basic.returns[game->basic.returnStack].sentence;
    }
}

// ゲームがクラシックかどうかを判定する
//
bool GameIsClassic(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    return game != NULL ? game->classic : true;
}

// 機能が有効かどうかを判定する
//
bool GameIsShortRangeSensorAvilable(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    return game != NULL && game->basic.at[1 + 63] == 0 ? true : false;
}
bool GameIsLongRangeSensorAvilable(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    return game != NULL && game->basic.at[3 + 63] == 0 ? true : false;
}
bool GameIsComputerDisplayAvilable(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    return game != NULL && game->basic.at[2 + 63] == 0 ? true : false;
}

// 地図なしを描画する
//
void GamePrintNoMap(LCDBitmap *bitmap)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 幅の取得
    int width0 = IocsGetTextWidth(kIocsFontJapanese, "9");
    int width1 = IocsGetTextWidth(kIocsFontMini, "9");

    // 高さの取得
    int height0 = IocsGetFontHeight(kIocsFontJapanese);
    int height1 = IocsGetFontHeight(kIocsFontMini);

    // 横の取得
    int x = 0;

    // 描画の開始
    playdate->graphics->pushContext(bitmap);
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->clearBitmap(bitmap, kColorBlack);

    // タイトルの描画
    {
        char *text = "NO MAP";
        IocsSetFont(kIocsFontMini);
        playdate->graphics->setDrawMode(kDrawModeFillWhite);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, 0 * height0 + 0 * height1);
        playdate->graphics->setDrawMode(kDrawModeCopy);
        playdate->graphics->drawLine(x, 0 * height0 + 1 * height1, x + 33 * width0 - 1 * width1, 0 * height0 + 1 * height1, 1, kColorWhite);
    }

    // 描画の完了
    playdate->graphics->popContext();
}

// 銀河系地図を描画する
//
void GamePrintGalaxyMap(LCDBitmap *bitmap)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ゲームの取得
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game == NULL) {
        return;
    }

    // 幅の取得
    int width0 = IocsGetTextWidth(kIocsFontJapanese, "9");
    int width1 = IocsGetTextWidth(kIocsFontMini, "9");

    // 高さの取得
    int height0 = IocsGetFontHeight(kIocsFontJapanese);
    int height1 = IocsGetFontHeight(kIocsFontMini);

    // 横の取得
    int x = 0;

    // 描画の開始
    playdate->graphics->pushContext(bitmap);
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->clearBitmap(bitmap, kColorBlack);

    // タイトルの描画
    {
        char *text = "GALAXY MAP";
        IocsSetFont(kIocsFontMini);
        playdate->graphics->setDrawMode(kDrawModeFillWhite);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, 0 * height0 + 0 * height1);
        playdate->graphics->setDrawMode(kDrawModeCopy);
        playdate->graphics->drawLine(x, 0 * height0 + 1 * height1, x + 33 * width0 - 1 * width1, 0 * height0 + 1 * height1, 1, kColorWhite);
    }

    // マップの描画
    {
        IocsSetFont(kIocsFontJapanese);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int x = (j * 4) * width0;
                int y = (i + 1) * height0;
                if (game->basic.U - 1 == i && game->basic.V - 1 == j) {
                    playdate->graphics->setDrawMode(kDrawModeCopy);
                    playdate->graphics->fillRect(x, y, 5 * width0, height0, kColorWhite);
                }
                int m = game->basic.at[i * 8 + j];
                if (m > 0) {
                    char *text;
                    playdate->system->formatString(&text, " %03d", m);
                    playdate->graphics->setDrawMode(kDrawModeXOR);
                    playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, y);
                    playdate->system->realloc(text, 0);
                } else if (game->basic.U - 1 == i && game->basic.V - 1 == j) {
                    char *text = " ???";
                    playdate->graphics->setDrawMode(kDrawModeXOR);
                    playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, y);
                } else {
                    char *text = " ...";
                    playdate->graphics->setDrawMode(kDrawModeXOR);
                    playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, y);
                }
            }
        }
    }

    // 描画の完了
    playdate->graphics->popContext();
}

// セクター地図を描画する
//
void GamePrintSectorMap(LCDBitmap *bitmap)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ゲームの取得
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game == NULL) {
        return;
    }

    // 幅の取得
    int width0 = IocsGetTextWidth(kIocsFontJapanese, "9");
    int width1 = IocsGetTextWidth(kIocsFontMini, "9");

    // 高さの取得
    int height0 = IocsGetFontHeight(kIocsFontJapanese);
    int height1 = IocsGetFontHeight(kIocsFontMini);

    // 横の取得
    int x = 0;

    // 描画の開始
    playdate->graphics->pushContext(bitmap);
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->clearBitmap(bitmap, kColorBlack);

    // タイトルの描画
    {
        char *text = "SECTOR MAP";
        IocsSetFont(kIocsFontMini);
        playdate->graphics->setDrawMode(kDrawModeFillWhite);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x, 0 * height0 + 0 * height1);
        playdate->graphics->setDrawMode(kDrawModeCopy);
        playdate->graphics->drawLine(x, 0 * height0 + 1 * height1, x + 33 * width0 - 1 * width1, 0 * height0 + 1 * height1, 1, kColorWhite);
    }

    // マップの描画
    {
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->setDrawMode(kDrawModeFillWhite);
        for (int i = 1; i <= 8; i++) {
            for (int j = 1; j <= 8; j++) {
                int x = ((j - 1) * 2 + 9) * width0;
                int y = ((i - 1) + 1) * height0;
                int m = game->basic.at[i * 8 + j + 62];
                static char *texts[] = {
                    ".", 
                    "K", 
                    "B", 
                    "*", 
                    "E", 
                };
                playdate->graphics->drawText(texts[m], strlen(texts[m]), kUTF8Encoding, x, y);
            }
        }
    }

    // 描画の完了
    playdate->graphics->popContext();
}

// レポートを描画する
//
void GamePrintReport(LCDBitmap *bitmap)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // ゲームの取得
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game == NULL) {
        return;
    }

    // 幅の取得
    char *border = "--------------------------------"; 
    int width = IocsGetTextWidth(kIocsFontMini, border);

    // 高さの取得
    int height0 = IocsGetFontHeight(kIocsFontJapanese);
    int height1 = IocsGetFontHeight(kIocsFontMini);

    // 横の取得
    int x0 = 0;
    int x1 = width - IocsGetTextWidth(kIocsFontJapanese, "999999");

    // 描画の開始
    playdate->graphics->pushContext(bitmap);
    playdate->graphics->setDrawMode(kDrawModeCopy);
    playdate->graphics->clearBitmap(bitmap, kColorBlack);
    playdate->graphics->setDrawMode(kDrawModeFillWhite);

    // タイトルの描画
    {
        char *text = "STATUS";
        IocsSetFont(kIocsFontMini);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 0 * height0 + 0 * height1);
        // playdate->graphics->drawText(border, strlen(border), kUTF8Encoding, x0, 0 * height0 + 1 * height1);
        playdate->graphics->drawLine(x0, 0 * height0 + 1 * height1, x0 + width - 1, 0 * height0 + 1 * height1, 1, kColorWhite);
    }

    // STARDATE の描画
    {
        char *text;
        playdate->system->formatString(&text, "STARDATE  %d", 3230 - game->basic.D);
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 1 * height0 + 0 * height1);
        playdate->system->realloc(text, 0);
    }

    // TIME LEFT の描画
    {
        char *text;
        playdate->system->formatString(&text, "TIMELEFT  %d", game->basic.D);
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 2 * height0 + 0 * height1);
        playdate->system->realloc(text, 0);
    }

    // CONDITION の描画
    {
        char *text = NULL;
        if (game->basic.O != 0) {
            text = "CONDITION DOCKED";
        } else if (game->basic.N != 0) {
            text = "CONDITION RED";
        } else if (game->basic.E < 999) {
            text = "CONDITION YELLOW";
        } else {
            text = "CONDITION GREEN";
        }
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 3 * height0 + 0 * height1);
    }

    // ENERGY の描画
    {
        char *text;
        playdate->system->formatString(&text, "ENERGY    %d", game->basic.E);
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 4 * height0 + 0 * height1);
        playdate->system->realloc(text, 0);
    }

    // TORPEDOES の描画
    {
        char *text;
        playdate->system->formatString(&text, "TORPEDOES %d", game->basic.F);
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 5 * height0 + 0 * height1);
        playdate->system->realloc(text, 0);
    }

    // KLINGONS LEFT の描画
    {
        char *text;
        playdate->system->formatString(&text, "KLINGONS  %d", game->basic.K);
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 6 * height0 + 0 * height1);
        playdate->system->realloc(text, 0);
    }

    // STARBASES の描画
    {
        char *text;
        playdate->system->formatString(&text, "STARBASES %d", game->basic.B);
        IocsSetFont(kIocsFontJapanese);
        playdate->graphics->drawText(text, strlen(text), kUTF8Encoding, x0, 7 * height0 + 0 * height1);
        playdate->system->realloc(text, 0);
    }

    // 描画の完了
    playdate->graphics->popContext();
}
