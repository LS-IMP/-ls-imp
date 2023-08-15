$$
\begin{table}[!ht]
\centering
\renewcommand\arraystretch{0.8}
% \small
\begin{threeparttable}
\caption{Inventory metrics between Gurobi and LS-IMP}
\label{Commercial}
\setlength{\tabcolsep}{2.0mm}\scalebox{0.9}{
\begin{tabular}{ccccccc}
\hline
    \multirow{3}{*}{Dataset}  & \multicolumn{4}{c}{Gurobi} & \multicolumn{2}{c}{\multirow{2}{*}{LS-IMP}} \\
\cmidrule(lr){2-5}

 &  \multicolumn{2}{c}{Exact} & \multicolumn{2}{c}{Heur} & \multicolumn{2}{c}{} \\
 
\cmidrule(lr){2-3}\cmidrule(lr){4-5}\cmidrule(lr){6-7}
 &  \#UR &\#FR & \#UR & \#FR & \#UR & \#FR \\
\hline
\multicolumn{7}{c}{cutoff=60s} \\
\hline
$\mathcal{D}_1$   & 0 & 0   & 0    & 0    & \textbf{15.5\%}    & \textbf{64.7\%}   \\
$\mathcal{D}_2$    & 0 & 0   & 1.7\%  & 5.7\%   & \textbf{16.4\%}   & \textbf{60.7\%}  \\
$\mathcal{D}_3$   & 0 & 0   & 2.3\%  & 9.0\%   & \textbf{16.7\%}  & \textbf{62.0\%}    \\
$\mathcal{D}_4$   & 0 & 0   & 1.2\%   & 5.0\%  & \textbf{16.3\%}   & \textbf{61.6\%}     \\
$\mathcal{D}_5$   & 0 & 0   & 1.8\%   & 4.8\%  & \textbf{17.0\%}   & \textbf{65.0\%}     \\[0.3ex] 
Total & 0 & 0   & 1.4\%     & 4.9\%   & \textbf{16.4\%}    & \textbf{62.8\%}   \\
\hline
\multicolumn{7}{c}{cutoff=300s} \\
\hline
$\mathcal{D}_1$    & 7.7\%    & 39.6\%   &  0.6\%    & 7.0\%   & \textbf{17.7\%}    & \textbf{68.8\%}   \\
$\mathcal{D}_2$  & 6.0\%  & 35.5\%    & 2.6\%   & 13.9\%    & \textbf{17.7\%}   & \textbf{62.8\%}  \\
$\mathcal{D}_3$    & 5.1\% & 22.8\%   & 4.3\%  &  14.7\%   & \textbf{18.1\%}  & \textbf{64.3\%}    \\
$\mathcal{D}_4$   & 5.9\% & 28.8\%    & 2.4\%   & 11.7\%   & \textbf{16.6\%}   & \textbf{62.5\%}     \\
$\mathcal{D}_5$    & 5.3\% & 24.7\%    & 3.4\%  &  11.2\%   & \textbf{17.3\%}   & \textbf{65.7\%}     \\[0.3ex] 
Total    & 6.0\% & 30.3\%    & 2.7\%  &  11.7\%   & \textbf{17.5\%}   & \textbf{64.8\%}     \\
\hline
\multicolumn{7}{c}{cutoff=1000s} \\
\hline
$\mathcal{D}_1$    & 15.3\%   &  62.3\%    & 6.6\%    &24.1\%    & \textbf{17.7\%}    & \textbf{68.9\%}   \\
$\mathcal{D}_2$  & 14.7\%  & 61.3\%    & 6.3\%   & 28.4\%    & \textbf{17.7\%}   & \textbf{62.9\%}  \\
$\mathcal{D}_3$   & 11.4\% & 46.8\%   & 7.0\%  &  25.2\%    & \textbf{18.1\%}  & \textbf{64.4\%}    \\
$\mathcal{D}_4$    & 12.1\% & 52.6\%   & 4.2\%  & 22.2\%   & \textbf{16.6\%}   & \textbf{62.6\%}     \\
$\mathcal{D}_5$   & 14.6\% & 55.6\%    & 6.8\%  &  23.8\%   & \textbf{17.4\%}   & \textbf{66.0\%}     \\[0.3ex] 
Total    & 13.6\%  & 55.7\%    &  6.2\%    & 24.7\%    & \textbf{17.5\%}    & \textbf{65.0\%}   \\
\hline
\end{tabular}
}
\end{threeparttable}
\end{table}
$$
