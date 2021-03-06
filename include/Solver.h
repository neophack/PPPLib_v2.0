//
// Created by cc on 7/23/20.
//

#ifndef PPPLIB_SOLVER_H
#define PPPLIB_SOLVER_H

#include "GnssFunc.h"
#include "GnssErrorModel.h"
#include "InsFunc.h"
#include "AdjFunc.h"
#include "OutSol.h"

using namespace Eigen;
namespace PPPLib{

    class cSolver {
    public:
        cSolver();
        virtual ~cSolver();

    public:
        void InitEpochSatInfo(vector<tSatInfoUnit>& sat_infos);
        void UpdateSatInfo(vector<tSatInfoUnit>& sat_infos);
        void UpdateGnssObs(tPPPLibConf C,tEpochSatUnit& epoch_sat,RECEIVER_INDEX rec);
        void CorrGnssObs(tPPPLibConf C);
        virtual int GnssObsRes(int post,tPPPLibConf C,double* x);

        virtual void InitSolver(tPPPLibConf C);
        virtual bool SolverProcess(tPPPLibConf C);
        virtual bool SolverEpoch();
        virtual bool Estimator(tPPPLibConf C);
        virtual bool SolutionUpdate();

        void InitFullPx(tPPPLibConf C);
        void InitX(double xi,double var,int idx,double *x,double *xp);
        Eigen::MatrixXd InitQ(tPPPLibConf,double dt);

    public:
        cGnssObsOperator gnss_obs_operator_;
        cGnssErrCorr gnss_err_corr_;
        cParSetting para_;
        cLsqAdjuster lsq_;
        cKfAdjuster kf_;
        cOutSol *out_;

    public:
        int epoch_idx_=0;
        int epoch_ok_=0;
        int epoch_fail_=0;
        tNav nav_;
        cGnssObs rover_obs_;
        vector<tSolInfoUnit> ref_sols_;

        tEpochSatUnit epoch_sat_obs_;
        vector<tSatInfoUnit> epoch_sat_info_collect_;
        vector<tSatInfoUnit> base_sat_info_collect_;

        tSolInfoUnit ppplib_sol_;
        vector<tSolInfoUnit> sol_collect_;
        tSatInfoUnit previous_sat_info_[MAX_SAT_NUM];

    public:
        int num_full_x_,num_zip_x_;
        VectorXd full_x_;
        MatrixXd full_Px_;
        int num_valid_sat_;
        int num_L_;
        VectorXd omc_L_;
        MatrixXd H_;
        MatrixXd R_;
        MatrixXd F_; //for coupled
        int sys_mask_[NSYS]={0};
    };

    class cSppSolver:public cSolver {
    public:
        cSppSolver();
        cSppSolver(tPPPLibConf conf);
        ~cSppSolver();

    private:
        void CorrDoppler(tSatInfoUnit& sat_info, Vector3d& rover_xyz,int f);
        Vector3d SatVelSagnacCorr(const Vector3d& sat_pos,const double tau);
        int DopplerRes(tPPPLibConf C,MatrixXd& H_mat, MatrixXd& R_mat,VectorXd& L,VectorXd& x,Vector3d rover_xyz);
        void EstDopVel(Vector3d rover_xyz);

        double Dops();
        bool ValidateSol(tPPPLibConf C);


    public:
        int GnssObsRes(int post,tPPPLibConf C,double* x) override;
        void InitSolver(tPPPLibConf C) override;
        bool SolverProcess(tPPPLibConf C) override;
        bool SolverEpoch() override;
        bool Estimator(tPPPLibConf C) override;
        bool SolutionUpdate() override;

    private:
        int iter_=10;
    public:
        tPPPLibConf spp_conf_;
    };

    class cPppSolver:public cSolver {
    public:
        cPppSolver();
        cPppSolver(tPPPLibConf C);
        ~cPppSolver();

    public:
        void InitSolver(tPPPLibConf C) override;
        bool SolverProcess(tPPPLibConf C) override;
        bool SolverEpoch() override;
        bool Estimator(tPPPLibConf C) override;
        bool SolutionUpdate() override;

    private:
        void InitSppSolver();
        void Spp2Ppp();
        void PppCycSlip(tPPPLibConf C);
        void PosUpdate(tPPPLibConf C);
        void ClkUpdate(tPPPLibConf C,double tt);
        void TrpUpdate(tPPPLibConf C,double tt);
        void IonUpdate(tPPPLibConf C,double tt);
        void AmbUpdate(tPPPLibConf C,double tt);
        void StateTimeUpdate(tPPPLibConf C);
        int GnssObsRes(int post,tPPPLibConf C,double *x) override;


    private:
        tPPPLibConf ppp_conf_;
        int max_iter_=8;

    public:
        cSppSolver *spp_solver_;
    };

    class cPpkSolver:public cSolver {
    public:
        cPpkSolver();
        cPpkSolver(tPPPLibConf C);
        ~cPpkSolver();

    public:
        void InitSolver(tPPPLibConf C) override;
        bool SolverProcess(tPPPLibConf C) override;
        bool SolverEpoch() override;
        bool Estimator(tPPPLibConf C) override;
        bool SolutionUpdate() override;

    private:
        void InitSppSolver();
        void Spp2Ppk();
        bool GnssZeroRes(tPPPLibConf C,RECEIVER_INDEX rec,vector<int>sat_idx,double* x);
        int GnssDdRes(int post,tPPPLibConf C,vector<int>ir,vector<int>ib,vector<int>cmn_sat_no,double* x);
        bool ValidObs(int i,int nf,int f);
        bool MatchBaseObs(cTime t);
        int SelectCmnSat(tPPPLibConf C,vector<int>& ir,vector<int>& iu,vector<int>& cmn_sat_no);
        void PpkCycleSlip(tPPPLibConf C,vector<int>& iu,vector<int>& ib,vector<int>& cmn_sat_no);
        void StateTimeUpdate(tPPPLibConf C,vector<int>& iu,vector<int>& ib,vector<int>& cmn_sat_no);
        void PosUpdate(tPPPLibConf C);
        void TrpUpdate(tPPPLibConf C,double tt);
        void IonUpdate(tPPPLibConf C,double tt);
        void AmbUpdate(tPPPLibConf C, double tt,vector<int>& ir,vector<int>& ib,vector<int>& cmn_sat_no);

    private:
        cGnssObs base_obs_;
        cSppSolver *spp_solver_;
        tPPPLibConf ppk_conf_;
        Vector3d base_xyz_;

    private:
        tEpochSatUnit base_epoch_sat_obs_;
        int base_idx_=0;
        vector<double>base_res,rover_res;
    };

    class cFusionSolver:public cSolver {
    public:
        cFusionSolver();
        cFusionSolver(tPPPLibConf C);
        ~cFusionSolver();

    private:
        bool InputImuData(int ws);
        bool MatchGnssObs();

        double Vel2Yaw(Vector3d vn);
        bool GnssSol2Ins(Vector3d re,Vector3d ve);
        Vector3d Pos2Vel(tSolInfoUnit& sol1,tSolInfoUnit& sol2);
        bool InsAlign();

    public:
        void InitSolver(tPPPLibConf C) override;
        bool SolverProcess(tPPPLibConf C) override;
        bool SolverEpoch() override;
        bool SolutionUpdate() override;

    private:
        void CloseLoopState();
        void StateTimeUpdate();
        void RemoveLever(const Vector3d& ins_re,const Vector3d& ins_ve,Vector3d& lever,Vector3d& gnss_re,Vector3d& gnss_ve);
        int BuildLcHVR(int post,tPPPLibConf C,Vector3d ins_re,Vector3d ins_ve,double *meas_pos,double *meas_vel);
        bool LcFilter(tPPPLibConf C);

    public:
        bool LooseCouple(tPPPLibConf C);
        bool TightCouple(tPPPLibConf C);

    private:
        cInsMech ins_mech_;
        cSolver *gnss_solver_;
        tPPPLibConf fs_conf_;

    private:
        cImuData imu_data_;

    private:
        int imu_index_=0;
        int rover_idx_=0;
        tImuInfoUnit cur_imu_info_={0};
        tImuInfoUnit pre_imu_info_={0};
        vector<tImuDataUnit> imu_data_zd_;
    };
}




#endif //PPPLIB_SOLVER_H
