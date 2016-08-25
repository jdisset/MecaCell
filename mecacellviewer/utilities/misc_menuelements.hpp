/*
    cellsMenu.at("Display basis").onToggled = [&](R *r, MenuElement<R> *me) {
      if (me->isChecked()) {
        paintStepsMethods[18] = [&](R *r) {
          const double lx = 15.0;
          const double ly = 15.0;
          ArrowsGroup<R> *arrows =
              dynamic_cast<ArrowsGroup<R> *>(paintSteps["Arrows"].get());
          vector<std::pair<QVector3D, QQuaternion>> basis;
          basis.reserve(r->scenario.getWorld().cells.size());
          for (auto &c : r->scenario.getWorld().cells) {
            QQuaternion qq =
                QQuaternion::fromAxisAndAngle(toQV3D(c->getOrientationRotation().n),
                                              radToDeg(c->getOrientationRotation().teta));
            basis.push_back(make_pair(toQV3D(c->getPosition()), qq));
          }
          vector<pair<QVector3D, QVector3D>> f;
          for (auto &b : basis) {
            f.push_back(
                make_pair(b.first, b.second.rotatedVector(QVector3D(1, 0, 0)) * lx));
          }
          arrows->call(r, f, QVector4D(1.0, 0.1, 0.3, 1.0));
          f.clear();
          for (auto &b : basis) {
            f.push_back(
                make_pair(b.first, b.second.rotatedVector(QVector3D(0, 1, 0)) * lx));
          }
          arrows->call(r, f, QVector4D(0.1, 0.3, 1.0, 1.0));
        };
      } else {
        paintStepsMethods.erase(18);
      }
    };
    cellsMenu.at("Display forces").onToggled = [&](R *r, MenuElement<R> *me) {
      if (me->isChecked()) {
        paintStepsMethods[15] = [&](R *r) {
          ArrowsGroup<R> *arrows =
              dynamic_cast<ArrowsGroup<R> *>(paintSteps["Arrows"].get());
          auto f0 = r->scenario.getWorld().getAllForces();
          vector<pair<QVector3D, QVector3D>> f;
          f.reserve(f0.size());
          for (auto &p : f0) {
            f.push_back(make_pair(toQV3D(p.first), toQV3D(p.second)));
          }
          arrows->call(r, f, QVector4D(1.0, 0.3, 0.6, 1.0));
        };
      } else {
        paintStepsMethods.erase(15);
      }
    };
    MenuElement<R> screenCap = {"Screen capture", false};
    screenCap.onToggled = [&](R *r, MenuElement<R> *me) {
      if (me->isChecked()) {
        paintStepsMethods[1900000] = [&](R *r) { paintSteps["ScreenCapture"]->call(r); };
      } else {
        paintStepsMethods.erase(1900000);
      }
    };
*/
