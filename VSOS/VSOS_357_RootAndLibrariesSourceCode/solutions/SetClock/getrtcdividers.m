len=1000;
ok=1000/32768
q=zeros(4,len);
for i=1:len,
  q(i,1)=i;
  q(i,2)=round(i/ok/2);
  q(i,3)=q(i,1)/q(i,2)/2;
  q(i,4)=abs(q(i,3)-ok);
end
[sr,id]=sort(q(:,4));
%id=1:len;
for i=1:len,
  idx = id(i);
  if q(idx,2) < 4096,
    printf("tCAdd %4d / %5d+1, res %8.6f, err %10.8f -> %dPPM\n", q(idx,1), q(idx,2)-1, q(idx,3), q(idx,4), q(idx,4)/ok*1000000);
  endif
endfor
