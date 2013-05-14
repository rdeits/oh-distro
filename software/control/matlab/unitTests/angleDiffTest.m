function angleDiffTest()

  for j = 1:100
    phi1 = random('unif', -3*pi, 3*pi);
    theta = random('unif', -pi, pi);
    phi2 = phi1 + theta;
    d = angleDiff(phi1, phi2);
    if abs(d - theta) > 1e-3 || abs(d - diff(unwrap([phi1, phi2]))) > 1e-3
      phi1
      phi2
      theta
      d
      unwrap(phi1, phi2)
      diff(unwrap(phi1, phi2))
      error('angle diff unit test failed')
    end
  end