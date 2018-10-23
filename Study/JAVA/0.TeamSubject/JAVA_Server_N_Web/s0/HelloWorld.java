
public class HelloWorld {
	public static void main (String[] args) {
		int Towers[] = new int[] { 5, 2, 3, 6, 1, 2, 3 };
		
		System.out.println("ans ->: " + cntSeenTower2(Towers, 0, Towers.length - 1));
		System.out.println("ans <-: " + cntSeenTower2(Towers, Towers.length - 1, 0));
	}
	
	public static int cntSeenTower2(int towerHeights[], int start, int end) {
		int cnt = 1;
		int dtEnd = end - start;
		boolean isLow2High = ((dtEnd > 0) ? true : false);		// 0 -> length (true) || 0 <- length (false)
		int maxHeight = towerHeights[(isLow2High ? 0 : end)];
		
		int loopI = (isLow2High ? start + 1 : end - 1);

		while ((isLow2High ? loopI <= end : loopI >= 0)) {
			if (maxHeight < towerHeights[loopI]) {
				maxHeight = towerHeights[loopI];
				++cnt;
			}
			
			loopI = (isLow2High ? loopI + 1 : loopI - 1);
		}
		
		return cnt;
	}
}