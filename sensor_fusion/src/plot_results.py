import pandas as pd
import matplotlib.pyplot as plt
import os

#データの読み込み
log_file = 'logs/sensor_log.csv'
if not os.path.exists(log_file):
    print(f"【ERROR】: {log_file} が見つかりません。先に ./navi_sim を実行してください。")
    exit()

df = pd.read_csv(log_file)

#グラフ全体のスタイル設定
plt.style.use('seaborn-v0_8-darkgrid')

#グラフ1：速度の比較（真値 vs 推定値）
plt.figure(figsize=(12, 6))
plt.subplot(1, 2, 1)
plt.plot(df['Step'], df['Virtual_speed'], label='True Speed', color='gray', linestyle='--')
plt.plot(df['Step'], df['Fusion_Speed'], label='Fused Speed', color='blue', linewidth=2)
plt.title('Speed Estimation (km/h)')
plt.xlabel('Step')
plt.ylabel('Speed')
plt.legend()

#グラフ2：航跡（X, Y座標の動き）
plt.subplot(1, 2, 2)
plt.plot(df['X'], df['Y'], label='Estimated Path', color='red', marker='o', markersize=2)
plt.title('Vehicle Trajectory (m)')
plt.xlabel('X (m)')
plt.ylabel('Y (m)')
plt.axis('equal') # 縦横比を1:1に固定
plt.legend()

plt.tight_layout()

#画像として保存
output_plot = 'logs/simulation_result.png'
plt.savefig(output_plot)
print(f"【SUCCESS】: グラフを {output_plot} に保存しました。")

# 統計情報の表示
print("\n--- シミュレーション統計 ---")
print(f"平均誤差: {df['Error'].abs().mean():.4f} km/h")
print(f"最大誤差: {df['Error'].abs().max():.4f} km/h")
